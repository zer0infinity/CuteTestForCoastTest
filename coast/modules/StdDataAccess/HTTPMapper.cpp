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
#include "Timers.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

static void SuppressListToLower(ROAnything suppressList, Anything &suppressListToLower)
{
	const long size = suppressList.GetSize();
	for (long i = 0; i < size; i++) {
		if (suppressList[i].GetType() == Anything::eArray) {
			SuppressListToLower(suppressList[i], suppressListToLower);
		} else {
			String tmp;
			tmp = suppressList[i].AsCharPtr();
			tmp.ToLower();
			suppressListToLower[tmp] = 1L;
		}
	}
}

/* ************************* Removed to prove obsoleteness, 16.1.2004, kvg

//--- HTTPHeaderMapper ---------------------------
RegisterOutputMapper(HTTPHeaderResultMapper);
RegisterOutputMapperAlias(HTTPHeaderMapper,HTTPHeaderResultMapper);

bool HTTPHeaderResultMapper::DoPutStream(const char *, istream &is, Context &ctx,  ROAnything config)
{
	StartTrace(HTTPHeaderResultMapper.DoPutStream);
	DAAccessTimer(HTTPHeaderResultMapper.DoPutStream, "", ctx);

	String line ;
	String s;
	long status= 0;

	// read part by part from stream and put results into store using client mapper
	if ( !!is) {
		Trace("expecting HTTP-Version...");
		is >> s;
		Trace("got <" << s << ">");						// regular HTTP, just a version
		DoFinalPutAny("HTTP-Version", s, ctx);
	} else return false;

	if ( !!is) {
		Trace("expecting ResponseCode...");
		is >> status;
		Trace("got " << status);
		DoFinalPutAny("ResponseCode", status, ctx);
	} else return false;

	if ( !!is) {
		Trace("expecting ResponseMsg...");
		s= "";
		getline(is, s, '\n');			// status message
		if ('\r' == s[s.Length()-1L])
			s.Trim(s.Length()-1);			// remove last char (cr, HTTP uses crlf to separat lines)
		s.TrimFront(1);
		Trace("got " << s);
		DoFinalPutAny("ResponseMsg", s, ctx);
		Trace("Status message: " << s);
	} else return false;

	// get suppress list (if there is any)
	Anything suppresslist;
	SuppressListToLower(config["Suppress"], suppresslist);

	while (is.good() && ((getline(is,line, '\n'),line)!="" ) && (line != "\r") )
	// depends on how \r\n is treated
	{
		// still header
		Trace("line read: " << line);			// status message
		if ('\r' == line[line.Length()-1L])
			line.Trim(line.Length()-1);

		String fieldname;
		String item;
		long pos = 0;

		pos = line.StrChr(':');
		if (pos > 0) {
			// Assumed regular, a line with :
			fieldname = line.SubString(0,pos);
			fieldname.ToLower();	// all header fields stored in lower case
			item= line.SubString(pos+2);

			if ( ! suppresslist.IsDefined(fieldname) )
			{
				Trace("field name: <" << fieldname << ">");
				Trace("value: <" << item << ">");

				if (!DoFinalPutAny(fieldname, item, ctx))
					return false;
			}
			else
			{
				Trace("FILTER : " << fieldname << ": " << item);
			}
		}
		else
		{
			if (!DoFinalPutAny("otherLines", line, ctx))
				return false;
		}
	} // while

	return true;
}
*/

RegisterInputMapper(HTTPHeaderParameterMapper);

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
		for (long i = 0; i < headerfields.GetSize(); i++) {
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
				os << slotname << ": ";
				Trace("Header[" << slotname << "]=<" << rvalue.AsCharPtr() << ">");
				long elSz = rvalue.GetSize();
				for (long j = 0; j < elSz; j++) {
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
		}
	} else {
		TraceAny(ctx.GetTmpStore(), "no headers, get ReqHeader in tmp store:");
		String strHeaderfields;
		if (mapSuccess = Get("ReqHeader", strHeaderfields, ctx)) {
			os << strHeaderfields;
		}
	}
	Trace("retval: " << mapSuccess);
	return mapSuccess;
}

//--- HTTPBodyResultMapper ---------------------------
RegisterOutputMapper(HTTPBodyResultMapper);
RegisterOutputMapperAlias(HTTPBodyMapper, HTTPBodyResultMapper);

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

RegisterInputMapper(HTTPBodyParameterMapper);
RegisterInputMapperAlias(HTTPBodyMapper, HTTPBodyParameterMapper);
bool HTTPBodyParameterMapper::DoFinalGetStream(const char *key, ostream &os, Context &ctx)
{
	StartTrace1(HTTPBodyParameterMapper.DoFinalGetStream, NotNull(key));

	ROAnything params(ctx.Lookup(key)); //fixme??: use Get(key,any,ctx) instead?
	bool mapSuccess = true;

	if ( !params.IsNull() ) {
		// map a configured set of params
		long bPSz = params.GetSize();
		for (long i = 0; i < bPSz; i++) {
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
		if (mapSuccess = Get(key, bodyParams, ctx)) {
			os << bodyParams;
		}
	}
	Trace("retval: " << mapSuccess);
	return mapSuccess;
}
