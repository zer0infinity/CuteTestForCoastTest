/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPMapper.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "RequestProcessor.h"
#include "Timers.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

static void SuppressListToLower(ROAnything suppressList, Anything &suppressListToLower)
{
	const long size = suppressList.GetSize();
	for (long i = 0; i < size; ++i) {
		if (suppressList[i].GetType() == AnyArrayType) {
			SuppressListToLower(suppressList[i], suppressListToLower);
		} else {
			String tmp;
			tmp = suppressList[i].AsCharPtr();
			tmp.ToLower();
			suppressListToLower[tmp] = 1L;
		}
	}
}

RegisterParameterMapper(HTTPHeaderParameterMapper);

void HTTPHeaderParameterMapper::HandleOneLineForHeaderField(ostream &os, const String &slotname, ROAnything rvalue)
{
	StartTrace(HTTPHeaderParameterMapper.HandleOneLineForHeaderFields);

	os << slotname << ": ";
	Trace("Header[" << slotname << "]=<" << rvalue.AsCharPtr() << ">");
	long elSz = rvalue.GetSize();
	for (long j = 0; j < elSz; ++j) {
		if ( slotname == "COOKIE" ) {
			os << NotNull(rvalue.SlotName(j)) << '=';
		}
		os << rvalue[j].AsCharPtr("");
		if (j < (elSz - 1)) {
			os << (( slotname == "COOKIE" ) ? "; " : ", ");
		}
	}
	os << ENDL;
}

bool HTTPHeaderParameterMapper::HandleMoreLinesForHeaderField(ostream &os, const String &slotname, ROAnything rvalue)
{
	StartTrace(HTTPHeaderParameterMapper.HandleOneLineForHeaderFields);

	Trace("Header[" << slotname << "]=<" << rvalue.AsCharPtr() << ">");
	long elSz = rvalue.GetSize();
	bool handled = false;
	for (long j = 0; j < elSz; ++j) {
		if ( slotname == "SET-COOKIE" ) {
			handled = true;
			os << slotname << ": "  << rvalue[j].AsCharPtr("") << ENDL;
		}
	}
	return handled;
}

bool HTTPHeaderParameterMapper::DoGetStream(const char *key, ostream &os, Context &ctx,  ROAnything info)
{
	StartTrace1(HTTPHeaderParameterMapper.DoGetStream, "Key:<" << NotNull(key) << ">");

	ROAnything headerfields(ctx.Lookup(key));

	Anything suppresslist;
	SuppressListToLower(info["Suppress"], suppresslist);

	bool mapSuccess = true;
	TraceAny(headerfields, "Headerfields for lookup with " << key);

	if ( !headerfields.IsNull() ) {
		// map a configured set of headerfields
		for (long i = 0, szh = headerfields.GetSize(); i < szh; ++i) {
			String slotname = headerfields.SlotName(i);
			Anything value;
			ROAnything rvalue;

			String slotnameToLower = slotname;
			slotnameToLower.ToLower();

			if (slotnameToLower.Length() && !suppresslist.IsDefined(slotnameToLower)) {
				Trace("slot: " << slotname);
				if (!Get(slotname, value, ctx)) {
					rvalue = headerfields[slotname];
				} else {
					rvalue = value;
				}
				if ( !HandleMoreLinesForHeaderField(os, slotname, rvalue) ) {
					HandleOneLineForHeaderField(os, slotname, rvalue);
				}
			}
		}
	} else {
		TraceAny(ctx.GetTmpStore(), "no headers, get ReqHeader in tmp store:");
		String strHeaderfields;
		if ( ( mapSuccess = Get("ReqHeader", strHeaderfields, ctx) ) ) {
			os << strHeaderfields;
		}
	}
	String requestURI(ctx.Lookup("REQUEST_URI", ""));
	TraceAny(ctx.GetRequest(), "request:");
	if (		info.IsDefined("DoRedirectToOnLocationContent") &&
				info["DoRedirectToOnLocationContent"].IsDefined("OnLocationContent") &&
				info["DoRedirectToOnLocationContent"].IsDefined("RedirectTo") &&
				requestURI.Contains(info["DoRedirectToOnLocationContent"]["OnLocationContent"].AsString()) > -1) {
		String requestMethod(ctx.Lookup("REQUEST_METHOD", ""));
		if (		!info["DoRedirectToOnLocationContent"].IsDefined("OnMethod") ||
					info["DoRedirectToOnLocationContent"]["OnMethod"].AsString().IsEqual(requestMethod)) {
			String contentLength(ctx.Lookup("header.CONTENT-LENGTH", ""));
			if (		!info["DoRedirectToOnLocationContent"].IsDefined("OnContentLength") ||
						info["DoRedirectToOnLocationContent"]["OnContentLength"].AsString().IsEqual(contentLength) ) {

				RequestProcessor::ForceConnectionClose(ctx);
				ostream *ost = ctx.GetStream();
				ctx.GetTmpStore()["DirectMap"] = 1;

				String location = info["DoRedirectToOnLocationContent"]["RedirectTo"].AsString();

				(*ost) << "HTTP/1.x 302 Found" << ENDL << "Location: " << location << ENDL << "Connection: close" << ENDL << "Content-type: text/html" << ENDL << ENDL;
				(*ost) << "Redirect to: <a href=" << location << ">" << location << "</a>" << ENDL;
			}
		}
	}

	Trace("retval: " << mapSuccess);
	return mapSuccess;
}

//--- HTTPBodyResultMapper ---------------------------
RegisterResultMapper(HTTPBodyResultMapper);
RegisterResultMapperAlias(HTTPBodyMapper, HTTPBodyResultMapper);

bool HTTPBodyResultMapper::DoFinalPutStream(const char *key, istream &is, Context &ctx)
{
	StartTrace(HTTPBodyResultMapper.DoFinalPutStream);
	DAAccessTimer(HTTPBodyResultMapper.DoFinalPutStream, "", ctx);

	String body;

	ReadBody(body, is, ctx);
	return DoFinalPutAny(key, body, ctx);
}

void HTTPBodyResultMapper::ReadBody(String &body, istream &is, Context &ctx)
{
	StartTrace(HTTPBodyResultMapper.ReadBody);

	long contentLength = ctx.Lookup("Mapper.content-length", -1L);
	Trace("contentLength: " << contentLength);
	if (contentLength > -1) {
		body.Append(is, contentLength);
	} else {
		char c;
		while ( is.get(c).good() ) {
			body.Append(c);
		}
	}
	Trace("Body[" << body.Length() << "]");
	Trace("<" << body << ">");
}

RegisterParameterMapper(HTTPBodyParameterMapper);
RegisterParameterMapperAlias(HTTPBodyMapper, HTTPBodyParameterMapper);
bool HTTPBodyParameterMapper::DoFinalGetStream(const char *key, ostream &os, Context &ctx)
{
	StartTrace1(HTTPBodyParameterMapper.DoFinalGetStream, NotNull(key));

	ROAnything params(ctx.Lookup(key)); //!@FIXME ??: use Get(key,any,ctx) instead?
	bool mapSuccess = true;

	if ( !params.IsNull() ) {
		// map a configured set of params
		long bPSz = params.GetSize();
		for (long i = 0; i < bPSz; ++i) {
			const char *lookupVal = params.SlotName(i);
			if (!lookupVal) {
				lookupVal = params[i].AsCharPtr("");
			}

			String value;
			if (lookupVal && (mapSuccess = Get(lookupVal, value, ctx))) {
				Trace("Param[" << lookupVal << "]=<" << value << ">");
				os << lookupVal << "=" << value;
				if (i <  (bPSz - 1)) {
					os << "&";
				}
			} else {
				mapSuccess = true;
				os << lookupVal;
			}
			value = "";
		}
	} else {
		String bodyParams;
		if ( ( mapSuccess = Get(key, bodyParams, ctx) ) ) {
			os << bodyParams;
		}
	}
	Trace("retval: " << mapSuccess);
	return mapSuccess;
}
