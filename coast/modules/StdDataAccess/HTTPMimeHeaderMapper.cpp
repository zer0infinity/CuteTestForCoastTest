/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPMimeHeaderMapper.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "MIMEHeader.h"
#include "RE.h"

//--- c-modules used -----------------------------------------------------------

//---- HTTPMimeHeaderMapper ------------------------------------------------------------------
RegisterResultMapper(HTTPMimeHeaderMapper);

HTTPMimeHeaderMapper::HTTPMimeHeaderMapper(const char *name)
	: EagerResultMapper(name)
{
	StartTrace(HTTPMimeHeaderMapper.Ctor);
}

IFAObject *HTTPMimeHeaderMapper::Clone() const
{
	return new HTTPMimeHeaderMapper(fName);
}

bool HTTPMimeHeaderMapper::DoPutStream(const char *, istream &is, Context &ctx,  ROAnything config)
{
	// Ignore key, store under Mapper.HTTPHeader by default
	StartTrace(HTTPMimeHeaderMapper.DoPutStream);
	TraceAny(config, "My Config");
	MIMEHeader::ProcessMode eProcMode = MIMEHeader::eDoSplitHeaderFields;
	if (config.IsDefined("DoNotSplitHeaderFields") && config["DoNotSplitHeaderFields"].AsBool(0) == true) {
		eProcMode = MIMEHeader::eDoNotSplitHeaderFields;
	}
	MIMEHeader mh(URLUtils::eDownshift, eProcMode);
	bool result = mh.DoReadHeader(is);

	if (result && is.good()) {
		Anything header(mh.GetInfo());
		TraceAny(header, "header");
		if (eProcMode == MIMEHeader::eDoSplitHeaderFields) {
			CorrectDateFormats(header);
		}
		if (config["StoreCookies"].AsLong(0)) {
			StoreCookies(header, ctx);
		}
		if (config.IsDefined("Suppress")) {
			ROAnything suppresslist(config["Suppress"]);
			SuppressHeaders(header, suppresslist);
		}
		if (config.IsDefined("Add")) {
			ROAnything addlist(config["Add"]);
			AddHeaders(header, addlist);
		}
		if (config.IsDefined("Substitute")) {
			ROAnything addlist(config["Substitute"]);
			Substitute(header, addlist);
		}
		result = DoFinalPutAny("HTTPHeader", header, ctx);
	}
	return result && is.good();
}

void HTTPMimeHeaderMapper::CorrectDateFormats(Anything &header)
{
	// we assume to-lower keys
	const char *keylist[] = {
		"if-modified-since",
		"if-range",
		"if-unmodified-since",
		"last-modified",
		"retry-after",
		"date",
		"expires"
	};
	for (int i = 0; i < (int)(sizeof(keylist) / sizeof(keylist[0])); ++i) {
		const char *key = keylist[i];
		if (header.IsDefined(key) && header[key].GetType() == AnyArrayType) {
			String newvalue;
			Anything oldvalue(header[key]);
			for (long j = 0L, sz = oldvalue.GetSize(); j < sz; ++j) {
				if (newvalue.Length() > 0) {
					newvalue.Append(", ");
				}
				newvalue.Append(oldvalue[j].AsString());
			}
			header[key] = newvalue;
		}
	}
}

void HTTPMimeHeaderMapper::SuppressHeaders(Anything &header, ROAnything &suppresslist)
{
	for (long i = 0, sz = suppresslist.GetSize(); i < sz; ++i) {
		String keytosuppress = suppresslist[i].AsCharPtr();
		keytosuppress.ToLower();
		if (header.IsDefined(keytosuppress)) {
			header.Remove(keytosuppress);
		}
	}
}

void HTTPMimeHeaderMapper::AddHeaders(Anything &header, ROAnything &addlist)
{
	for (long i = 0, sz = addlist.GetSize(); i < sz; ++i) {
		String key(addlist.SlotName(i));
		String hdr(addlist[i].AsString());
		// normalize key to lower-type (header info stays as is)
		key.ToLower();
		// overwrite, if existing
		header[key] = hdr;
	}
}

void HTTPMimeHeaderMapper::Substitute(Anything &header, ROAnything &addlist)
{
	StartTrace(HTTPMimeHeaderMapper.Substitute);

	String key;
	String pattern;
	String replacement;
	String slotName = addlist.SlotName(0);
	if (slotName.Length() > 0) {
		key = addlist["Key"].AsString();
		pattern = addlist["Pattern"].AsString();
		replacement = addlist["Replacement"].AsString();
		RE substRegex(pattern, RE::MATCH_ICASE);
		for (int i = 0; i < header.GetSize(); i++) {
			if (header.SlotName(i) == key) {
				header[i] = substRegex.Subst(header[i].AsString(), replacement);
			}
		}
	} else {
		for (int i = 0; i < addlist.GetSize(); i++) {
			key = addlist[i]["Key"].AsString();
			pattern = addlist[i]["Pattern"].AsString();
			replacement = addlist[i]["Replacement"].AsString();

			RE substRegex(pattern, RE::MATCH_ICASE);
			for (int i = 0; i < header.GetSize(); i++) {
				if (header.SlotName(i) == key) {
					header[i] = substRegex.Subst(header[i].AsString(), replacement);
				}
			}
		}
	}
}

void HTTPMimeHeaderMapper::StoreCookies(Anything &header, Context &ctx)
{
	StartTrace(HTTPMimeHeaderMapper.StoreCookies);

	String backendName = ctx.GetTmpStore()["Backend"]["Name"].AsString();
	String cookieEnd = ";";

	for (int i = 0; i < header.GetSize(); i++) {
		Trace("SlotName: " << header.SlotName(i));
		if (String(header.SlotName(i)).IsEqual("set-cookie")) {
			String cookie = header[i].AsString();
			int pos = cookie.Contains(cookieEnd);
			if (pos == -1) {
				continue;
			}
			cookie = cookie.SubString(0, pos + 1);
			int equalityPos = cookie.Contains("=");
			String cookieName = cookie.SubString(0, equalityPos);
			cookieName.TrimWhitespace();
			Trace("cookieName: " << cookieName);
			String cookieValue = cookie.SubString(equalityPos + 1, cookie.Length() - (equalityPos + 1) - 1);
			cookieValue.TrimWhitespace();
			Trace("cookieValue: " << cookieValue);

			if (!cookieValue.IsEqual("")) {
				ctx.GetSessionStore()["StoredCookies"][backendName]["Structured"][cookieName] = cookieValue;
			} else {
				ctx.GetSessionStore()["StoredCookies"][backendName]["Structured"].Remove(cookieName);
			}
		}
	}

	String cookies = "Cookie:";
	for (int i = 0; i < ctx.GetSessionStore()["StoredCookies"][backendName]["Structured"].GetSize(); i++) {
		cookies = cookies 	<< " "
				  << ctx.GetSessionStore()["StoredCookies"][backendName]["Structured"].SlotName(i)
				  << "=" << ctx.GetSessionStore()["StoredCookies"][backendName]["Structured"][i].AsString()
				  << ";";
	}

	ctx.GetSessionStore()["StoredCookies"][backendName]["Plain"] = cookies;
	TraceAny(ctx.GetSessionStore(), "SessionStore:")
}
