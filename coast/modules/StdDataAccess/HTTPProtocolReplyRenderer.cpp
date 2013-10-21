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
		map[505] = "HTTP Version not supported";
		map[504] = "Gateway Time-Out";
		map[503] = "Out of Resources";
		map[502] = "Bad Gateway";
		map[501] = "Not Implemented";
		map[500] = "Server Error";

		map[415] = "Unsupported Media Type";
		map[414] = "Request-URL Too Large";
		map[413] = "Request Entity Too Large";
		map[412] = "Precondition Failed";
		map[411] = "Length Required";
		map[410] = "Gone";
		map[409] = "Conflict";
		map[408] = "Request Time-Out";
		map[407] = "Proxy Authentication Required";
		map[406] = "Not Acceptable";
		map[405] = "Method Not Allowed";
		map[404] = "Not Found";
		map[403] = "Forbidden";
		map[402] = "Payment Required";
		map[401] = "Unauthorized";
		map[400] = "Bad Request";

		map[305] = "Use Proxy";
		map[304] = "Not Modified";
		map[303] = "See Other";
		map[302] = "Moved Temporarily";
		map[301] = "Moved Permanently";
		map[300] = "Multiple Choices";

		map[206] = "Partial Content";
		map[205] = "Reset Content";
		map[204] = "No Content";
		map[203] = "Non-Authoritative Information";
		map[202] = "Accepted";
		map[201] = "Created";
		map[200] = "OK";

		map[101] = "Switching Protocols";
		map[100] = "Continue";
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

	String httpVersion = Renderer::RenderToString(ctx, realConfig[Coast::HTTP::constants::protocolVersionSlotname]);
	if (not httpVersion.Length()) {
		httpVersion = "HTTP/1.1";
	}
	reply << httpVersion << ' ';

	long status = Renderer::RenderToString(ctx, realConfig[Coast::HTTP::constants::protocolCodeSlotname]).AsLong(-1L);
	if ( status < 0L ) {
		status = 200L;
	}
	Assert(status >= 100L && status < 600L);
	reply << status << ' ';

	if (realConfig.IsDefined(Coast::HTTP::constants::protocolMsgSlotname)) {
		Renderer::Render(reply, ctx, realConfig[Coast::HTTP::constants::protocolMsgSlotname]);
	} else {
		Renderer::Render(reply, ctx, Anything(DefaultReasonPhrase(status)));
	}
	reply << ENDL;

	//!@FIXME: side effects, remove!!
	Anything anyStatus = status;
	StorePutter::Operate(anyStatus, ctx, "Tmp", String("HTTPStatus.").Append(Coast::HTTP::constants::protocolCodeSlotname));
	if (httpVersion.IsEqual("HTTP/1.1")) {
		if (status >= 400 || realConfig["CloseConnection"].AsBool(false)) {
			RequestProcessor::ForceConnectionClose(ctx);
		}
		if (!RequestProcessor::KeepConnectionAlive(ctx)) {
			reply << "Connection: close" << ENDL;
		}
	}
}

const char *HTTPProtocolReplyRenderer::DefaultReasonPhrase(long status) {
	StartTrace(HTTPProtocolReplyRenderer.DefaultReasonPhrase);
	return ROAnything(fgStatusCodeMap)[status].AsCharPtr("Unknown Error");
}
