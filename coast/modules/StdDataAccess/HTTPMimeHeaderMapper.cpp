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
	Trace("Result is: " << result);
	if (result && is.good()) {
		Anything header(mh.GetInfo());
		TraceAny(header, "MIMEHeader.GetInfo() output");
		TraceAny(header, "header");
		if (eProcMode == MIMEHeader::eDoSplitHeaderFields) {
			CorrectDateFormats(header);
		}
		if (config.IsDefined("Suppress")) {
			ROAnything suppresslist(config["Suppress"]);
			SuppressHeaders(header, suppresslist);
		}
		if (config.IsDefined("Add")) {
			ROAnything addlist(config["Add"]);
			AddHeaders(header, addlist);
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
	for (int i = 0; i < (int)(sizeof(keylist) / sizeof(keylist[0])); i++) {
		const char *key = keylist[i];
		if (header.IsDefined(key) && header[key].GetType() == Anything::eArray) {
			String newvalue;
			Anything oldvalue(header[key]);
			for (long j = 0L; j < oldvalue.GetSize(); j ++) {
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
	for (long i = 0; i < suppresslist.GetSize(); i++) {
		String keytosuppress = suppresslist[i].AsCharPtr();
		keytosuppress.ToLower();
		if (header.IsDefined(keytosuppress)) {
			header.Remove(keytosuppress);
		}
	}
}

void HTTPMimeHeaderMapper::AddHeaders(Anything &header, ROAnything &addlist)
{
	for (long i = 0; i < addlist.GetSize(); i++) {
		String key(addlist.SlotName(i));
		String hdr(addlist[i].AsString());
		// normalize key to lower-type (header info stays as is)
		key.ToLower();
		// overwrite, if existing
		header[key] = hdr;
	}
}
