/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPProtocolReplyRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "RequestProcessor.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- HTTPProtocolReplyRenderer ---------------------------------------------------------------
RegisterRenderer(HTTPProtocolReplyRenderer);

Anything HTTPProtocolReplyRenderer::fgStatusCodeMap = InitStatusCodeMap();

HTTPProtocolReplyRenderer::HTTPProtocolReplyRenderer(const char *name) : Renderer(name) { }

HTTPProtocolReplyRenderer::~HTTPProtocolReplyRenderer() { }

void HTTPProtocolReplyRenderer::EvilSideEffects(Context &ctx, ROAnything &realConfig, const String &httpVersion, String statusStr, long status, std::ostream &reply)
{
	if (statusStr.AsLong(-1L) < 0) {
		ctx.GetTmpStore()["HTTPStatus"]["ResponseCode"] = 200L;
	}
	if (httpVersion.IsEqual("HTTP/1.1")) {
		if (status >= 400 || realConfig["CloseConnection"].AsBool(false)) {
			RequestProcessor::ForceConnectionClose(ctx);
		}
		if (!RequestProcessor::KeepConnectionAlive(ctx)) {
			reply << "Connection: close" << ENDL;
		}
	}
}

void HTTPProtocolReplyRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(HTTPProtocolReplyRenderer.RenderAll);

	ROAnything realConfig((config.IsNull()) ? ctx.Lookup("HTTPStatus") : config);

	String httpVersion(Renderer::RenderToString(ctx, realConfig["HTTPVersion"]));
	if (httpVersion.Length() == 0) {
		httpVersion = "HTTP/1.1";
	}
	reply << httpVersion << ' ';

	String statusStr(Renderer::RenderToString(ctx, realConfig["ResponseCode"]));
	long status = statusStr.AsLong(200L);
	Assert(status >= 100L && status < 600L);
	reply << status << ' ';

	ROAnything lookupAny;
	if (realConfig.IsDefined("ResponseMsg")) {
		Renderer::Render(reply, ctx, realConfig["ResponseMsg"]);
	} else {
		Renderer::Render(reply, ctx, Anything(DefaultReasonPhrase(status)));
	}

	reply << ENDL;

	EvilSideEffects(ctx, realConfig, httpVersion, statusStr, status, reply);
}

const char *HTTPProtocolReplyRenderer::DefaultReasonPhrase(long status)
{
	StartTrace(HTTPProtocolReplyRenderer.DefaultReasonPhrase);
	return fgStatusCodeMap[status].AsCharPtr("Unknown Error");
}

Anything HTTPProtocolReplyRenderer::InitStatusCodeMap()
{
	StartTrace(HTTPProtocolReplyRenderer.InitStatusCodeMap);
	Trace("Initializing the statusCodeMap ...");
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

	return map;
}
