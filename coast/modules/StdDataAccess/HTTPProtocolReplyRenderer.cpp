/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPProtocolReplyRenderer.h"
#include "RequestProcessor.h"
#include "Server.h"
#include "AnythingUtils.h"
#include "HTTPConstants.h"

RegisterRenderer(HTTPProtocolReplyRenderer);

namespace {
	Anything InitStatusCodeMap() {
		StartTrace(HTTPProtocolReplyRenderer.InitStatusCodeMap);
		Anything map;
		// current status code list can be found within this or an updated rfc:
		// https://tools.ietf.org/rfc/rfc2616.txt
		//- 1xx: Informational - Request received, continuing process
		map["100"] = "Continue";
		map["101"] = "Switching Protocols";
		//- 2xx: Success - The action was successfully received,
		//  understood, and accepted
		map["200"] = "OK";
		map["201"] = "Created";
		map["202"] = "Accepted";
		map["203"] = "Non-Authoritative Information";
		map["204"] = "No Content";
		map["205"] = "Reset Content";
		map["206"] = "Partial Content";
		//- 3xx: Redirection - Further action must be taken in order to
		//  complete the request
		map["300"] = "Multiple Choices";
		map["301"] = "Moved Permanently";
		map["302"] = "Found";
		map["303"] = "See Other";
		map["304"] = "Not Modified";
		map["305"] = "Use Proxy";
		map["307"] = "Temporary Redirect";
		//- 4xx: Client Error - The request contains bad syntax or cannot
		//  be fulfilled
		map["400"] = "Bad Request";
		map["401"] = "Unauthorized";
		map["402"] = "Payment Required";
		map["403"] = "Forbidden";
		map["404"] = "Not Found";
		map["405"] = "Method Not Allowed";
		map["406"] = "Not Acceptable";
		map["407"] = "Proxy Authentication Required";
		map["408"] = "Request Time-out";
		map["409"] = "Conflict";
		map["410"] = "Gone";
		map["411"] = "Length Required";
		map["412"] = "Precondition Failed";
		map["413"] = "Request Entity Too Large";
		map["414"] = "Request-URI Too Large";
		map["415"] = "Unsupported Media Type";
		map["416"] = "Requested range not satisfiable";
		map["417"] = "Expectation Failed";
		//- 5xx: Server Error - The server failed to fulfill an apparently
		//  valid request
		map["500"] = "Internal Server Error";
		map["501"] = "Not Implemented";
		map["502"] = "Bad Gateway";
		map["503"] = "Service Unavailable";
		map["504"] = "Gateway Time-out";
		map["505"] = "HTTP Version not supported";
		//- COAST Specific error messages

		TraceAny(map, "status codes");
		return map;
	}
	Anything fgStatusCodeMap = InitStatusCodeMap();
}

void HTTPProtocolReplyRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(HTTPProtocolReplyRenderer.RenderAll);

	ROAnything realConfig;
	if ( config.GetType() == AnyCharPtrType ) {
		realConfig = ctx.Lookup(config.AsString());
	} else {
		realConfig = (config.IsNull() ? ctx.Lookup("HTTPStatus") : config);
	}
	TraceAny(realConfig, "HTTPStatus config");
	String httpVersion = Renderer::RenderToString(ctx, realConfig[coast::http::constants::protocolVersionSlotname]);
	if (not httpVersion.Length()) {
		httpVersion = "HTTP/1.1";
	}
	String strFirstLine(128L);
	strFirstLine << httpVersion << ' ';
	long status = Renderer::RenderToString(ctx, realConfig[coast::http::constants::protocolCodeSlotname]).AsLong(-1L);
	if ( status < 0L ) {
		status = 200L;
	}
	Assert(status >= 100L && status < 600L);
	strFirstLine << status << ' ';

	if (realConfig.IsDefined(coast::http::constants::protocolMsgSlotname)) {
		Renderer::RenderOnString(strFirstLine, ctx, realConfig[coast::http::constants::protocolMsgSlotname]);
	} else {
		Renderer::RenderOnString(strFirstLine, ctx, Anything(DefaultReasonPhrase(status)));
	}
	Trace("ProtocolReplyLine[" << strFirstLine << "]");
	reply << strFirstLine << ENDL;

	//!@FIXME: side effects, remove!!
	Anything anyStatus = status;
	StorePutter::Operate(anyStatus, ctx, "Tmp", String("HTTPStatus.").Append(coast::http::constants::protocolCodeSlotname));
	if (httpVersion.IsEqual("HTTP/1.1")) {
		if (status >= 400 || realConfig["CloseConnection"].AsBool(false)) {
			RequestProcessor::ForceConnectionClose(ctx);
		}
		if (!RequestProcessor::KeepConnectionAlive(ctx)) {
			reply << "Connection: close" << ENDL;
		}
	}
}

const char *HTTPProtocolReplyRenderer::DefaultReasonPhrase(long const status) {
	StartTrace1(HTTPProtocolReplyRenderer.DefaultReasonPhrase, String().Append(status));
	return ROAnything(fgStatusCodeMap)[String().Append(status)].AsCharPtr("Unknown Error");
}
