/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPResponseMapper.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- HTTPResponseMapper ------------------------------------------------------------------
RegisterResultMapper(HTTPResponseMapper);

HTTPResponseMapper::HTTPResponseMapper(const char *name) : ResultMapper(name)
{
	StartTrace(HTTPResponseMapper.Ctor);
}

IFAObject *HTTPResponseMapper::Clone(Allocator *a) const
{
	return new (a) HTTPResponseMapper(fName);
}
static void TrimMessage(String &message)
{
	message.TrimFront(1); // cut blank
	if (message[message.Length()-1L] == '\r') {
		message.Trim(message.Length() - 1L);
	}
}

bool HTTPResponseMapper::DoPutStream(const char *key, istream &is, Context &ctx, ROAnything conf)
{
	StartTrace1(HTTPResponseMapper.DoPutStream, NotNull(key));
	// ignore the key, assume a correct response line. Ignore conf, too.

	Anything response;

	// get version
	String version;
	is >> version;
	if (is.good()) {
		response["HTTP-Version"] = version;

		// get code
		long code = 0;
		is >> code;
		if (is.good()) {
			response["Code"] = code;

			// get message
			String message;
			getline(is, message);
			if (is.good()) {
				TrimMessage(message);
				response["Msg"] = message;
			}
		}
	}

	TraceAny(response, "Parsed response is: ");
	ResultMapper::DoPutAny("HTTPResponse", response, ctx, conf);
	return (is.good() != 0);
}

