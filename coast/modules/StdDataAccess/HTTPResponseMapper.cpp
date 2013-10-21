/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "HTTPResponseMapper.h"
#include "Tracer.h"
#include "HTTPConstants.h"

RegisterResultMapper(HTTPResponseMapper);

namespace {
	void TrimMessage(String &message) {
		message.TrimWhitespace();
		if (message[message.Length() - 1L] == '\r') {
			message.Trim(message.Length() - 1L);
		}
	}
}

bool HTTPResponseMapper::DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything conf) {
	StartTrace1(HTTPResponseMapper.DoPutStream, NotNull(key));
	Anything response;
	String version;
	is >> version;
	if (is.good()) {
		response[coast::http::constants::protocolVersionSlotname] = version;
		long code = 0;
		is >> code;
		if (is.good()) {
			response[coast::http::constants::protocolCodeSlotname] = code;
			String message;
			getline(is, message);
			if (is.good()) {
				TrimMessage(message);
				response[coast::http::constants::protocolMsgSlotname] = message;
			}
		}
	}
	TraceAny(response, "Parsed response is: ");
	ResultMapper::DoPutAny("HTTPResponse", response, ctx, conf);
	return (is.good() != 0);
}
