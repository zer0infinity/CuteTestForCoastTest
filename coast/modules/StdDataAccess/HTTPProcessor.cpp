/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPProcessor.h"

//--- standard modules used ----------------------------------------------------
#include "Timers.h"
#include "HTTPPostRequestBodyParser.h"
#include "HTTPRequestReader.h"
#include "Server.h"
#include "HTTPProtocolReplyRenderer.h"
#include "AnyIterators.h"
#include "AppBooter.h"
#include "AppLog.h"

//--- c-library modules used ---------------------------------------------------

RegisterRequestProcessor(HTTPProcessor);

//:configure request processor with server object
void HTTPProcessor::Init(Server *server)
{
	if ( server ) {
		fRequestSizeLimit = server->Lookup("RequestSizeLimit", fRequestSizeLimit);
		fLineSizeLimit = server->Lookup("LineSizeLimit", fLineSizeLimit);
		fURISizeLimit = server->Lookup("URISizeLimit", fURISizeLimit);
		fCheckUrlEncodingOverride = server->Lookup("CheckUrlEncodingOverride", fCheckUrlEncodingOverride);
		fCheckUrlPathContainsUnsafeCharsOverride = server->Lookup("CheckUrlPathContainsUnsafeCharsOverride", fCheckUrlPathContainsUnsafeCharsOverride);
		fCheckUrlPathContainsUnsafeCharsAsciiOverride = server->Lookup("CheckUrlPathContainsUnsafeCharsAsciiOverride", fCheckUrlPathContainsUnsafeCharsAsciiOverride);
		fCheckUrlPathContainsUnsafeCharsDoNotCheckExtendedAscii = server->Lookup("CheckUrlPathContainsUnsafeCharsDoNotCheckExtendedAscii", fCheckUrlPathContainsUnsafeCharsDoNotCheckExtendedAscii);
		fCheckUrlArgEncodingOverride = server->Lookup("CheckUrlArgEncodingOverride", fCheckUrlArgEncodingOverride);
		fUrlExhaustiveDecode = server->Lookup("URLExhaustiveDecode", fUrlExhaustiveDecode);
		fFixDirectoryTraversial = server->Lookup("FixDirectoryTraversial", fFixDirectoryTraversial);
		fURLEncodeExclude = server->Lookup("URLEncodeExclude", fURLEncodeExclude);
		fCheckHeaderFields = server->Lookup("CheckHeaderFields", (long) fCheckHeaderFields);
		fRejectRequestsWithInvalidHeaders = server->Lookup("RejectRequestsWithInvalidHeaders", (long) fRejectRequestsWithInvalidHeaders);

	}
	RequestProcessor::Init(server);
}

//#define REQ_TRACING

void HTTPProcessor::DoReadInput(std::iostream &Ios, Context &ctx)
{
	StartTrace(HTTPProcessor.DoReadInput);

	MIMEHeader header; // no super header
	HTTPRequestReader reader(this, header);
	{
		MethodTimer(HTTPRequestReader.ReadRequest, "Reading request", ctx);
		if (! reader.ReadRequest(Ios, ctx.GetRequest()["ClientInfo"]) ) {
			return;    // this was an error that forbids to process any further
		}
	}
	MethodTimer(HTTPProcessor.DoReadInput, "Reading input", ctx);
	Anything request(reader.GetRequest());

	Anything args(ctx.GetRequest());
	args["env"] = request;
	args["query"] = Anything(Anything::ArrayMarker());
	ctx.PushRequest(args);

	// prepare the environment for the framework
	SetWDClientInfo(ctx);
	ReadRequestBody(Ios, request, header, ctx);

	SubTraceAny(request, request, "Arguments:");
}

void HTTPProcessor::ReadRequestBody(std::iostream &Ios, Anything &request, MIMEHeader &header, Context &ctx)
{
	StartTrace(HTTPProcessor.ReadRequestBody);
	if ( request["REQUEST_METHOD"] == "POST" ) {
		HTTPPostRequestBodyParser sm(header, Ios);
		sm.Parse();
		if (fCheckHeaderFields && header.AreSuspiciosHeadersPresent() ) {
			Anything erreanousRequest(ctx.GetRequest());
			ctx.GetTmpStore()["ReadRequestBodyError"] =
				DoLogError(400, "Possible SSL Renegotiation attack. A multipart mime header (in POST) contains a GET/POST request",
						   ctx.GetRequest()["REQUEST_URI"].AsString(), ctx.GetRequest()["ClientInfo"], "", erreanousRequest, "HTTPPostRequestBodyParser");
		}
		request["REQUEST_BODY"] = sm.GetContent();
		request["WHOLE_REQUEST_BODY"] = sm.GetUnparsedContent();

#ifdef REQ_TRACING
		SystemLog::WriteToStderr(String("Body: ") << request["REQUEST_BODY"] << "\n");
#endif
		TraceAny(request["REQUEST_BODY"], "Body");
		TraceAny(request["WHOLE_REQUEST_BODY"], "Whole Body");
	} else {
		request["header"]["CONTENT-LENGTH"] = 0L;
	}
}

void HTTPProcessor::SetWDClientInfo(Context &ctx)
{
	Anything args(ctx.GetRequest());
	long sz = args["ClientInfo"].GetSize();
	for (long i = 0; i < sz; ++i) {
		args["env"]["header"][args["ClientInfo"].SlotName(i)] = args["ClientInfo"][i];
	}
	args["env"]["header"]["REMOTE-ADDR"] = args["ClientInfo"]["REMOTE_ADDR"].AsCharPtr("localhost");
	args["env"]["header"]["HTTPS"] = args["ClientInfo"]["HTTPS"].AsBool(false);
}

Anything HTTPProcessor::ParseCookie(const String &line)
{
	StartTrace(HTTPProcessor.ParseCookie);
	Trace("Line: " << line);

	Anything cookies;
	StringTokenizer st(line, ';');
	String key, cookieVal;

	while (st.NextToken(cookieVal)) {
		Trace("CookieVal: <" << cookieVal << ">[" << cookieVal.Length() << "]");
		// remove leading blanks
		if ( cookieVal.Length() ) {
			while ( (' ' == cookieVal[0L]) && cookieVal.Length() ) {
				cookieVal.TrimFront(1);
			}

			URLUtils::Pair(cookieVal, '=', cookies);
		}
	}
	TraceAny(cookies, "cookies: ");
	return cookies;
}

void HTTPProcessor::DoProcessRequest(std::ostream &reply, Context &ctx)
{
	StartTrace(HTTPProcessor.DoProcessRequest);
	if (IsZipEncodingAcceptedByClient(ctx)) {
		ctx.GetTmpStore()["ClientAcceptsGzipEnc"] = 1L;
	}
	RequestProcessor::DoProcessRequest(reply, ctx);
}

bool HTTPProcessor::IsZipEncodingAcceptedByClient(Context &ctx)
{
	StartTrace(HTTPProcessor.IsZipEncodingAcceptedByClient);
	TraceAny(ctx.GetRequest(), "Request");
	ROAnything roaEncoding;
	if (!ctx.Lookup("DisableZipEncoding", 0L) && ctx.Lookup("header.ACCEPT-ENCODING", roaEncoding) ) {
		TraceAny(roaEncoding, "accepted encodings");
		AnyExtensions::LeafIterator<ROAnything> iter(roaEncoding);
		ROAnything roaCurrAny;
		while (iter.Next(roaCurrAny)) {
			String enc = roaCurrAny.AsString("---");
			enc.ToLower();
			if (enc.IsEqual("gzip")) {
				Trace("accepting gzip");
				return true;
			}
		}
	}
	return false;
}

bool HTTPProcessor::DoKeepConnectionAlive(Context &ctx)
{
	StartTrace(HTTPProcessor.DoKeepConnectionAlive);

	String protocol = ctx.Lookup("SERVER_PROTOCOL", "");
	String connection = ctx.Lookup("header.CONNECTION", "");
	connection.ToLower();
	Trace("Protocol [" << protocol << "] connection [" << connection << "]");
	bool keepAlive = protocol.IsEqual("HTTP/1.1") && connection.IsEqual("keep-alive");
	Trace("Keep connection alive: " << keepAlive ? "Yes" : "No");

	return keepAlive;
}

void HTTPProcessor::DoRenderProtocolStatus(std::ostream &os, Context &ctx)
{
	HTTPProtocolReplyRenderer r("HTTPProtocolReplyRenderer");
	r.RenderAll( os, ctx, ROAnything());
}

Anything HTTPProcessor::DoLogError(long errcode, const String &reason, const String &line, const Anything &clientInfo, const String &msg, Anything &request, const char *who)
{
	StartTrace(HTTPProcessor.DoLogError);
	TraceAny(clientInfo, "client info:");
	// define SecurityLog according to the AppLog rules if you want to see this output.
	Context ctx;
	Anything tmp = ctx.GetTmpStore();
	tmp[who]["REMOTE_ADDR"] = clientInfo["REMOTE_ADDR"];
	tmp[who]["HTTPS"] = clientInfo["HTTPS"];
	tmp[who]["Request"] = request;
	tmp[who]["HttpStatusCode"]	=  errcode;
	tmp[who]["HttpResponseMsg"] = msg;
	tmp[who]["Reason"] = reason;
	tmp[who]["FaultyRequestLine"] = line;
	fErrors[who] = tmp[who];
	Application *pApp = NULL;
	Server *pServer = NULL;
	String strServerName;
	pApp = AppBooter().FindApplication(Application::GetConfig(), strServerName);
	TraceAny(tmp[who], "global server, aka 'MasterServer', is [" << strServerName << "]");
	pServer = SafeCast(pApp, Server);
	if ( pServer ) {
		ctx.SetServer(pServer);
	}
	AppLogModule::Log(ctx, "SecurityLog", AppLogModule::eERROR);
	return tmp;
}

//! render the protocol specific error msg
void HTTPProcessor::DoError(std::ostream &reply, const String &msg, Context &ctx)
{
	ROAnything httpStatus = ctx.Lookup("HTTPStatus");
	long errorCode(httpStatus["ResponseCode"].AsLong(400L));
	String errorMsg(HTTPProtocolReplyRenderer::DefaultReasonPhrase(errorCode));
	DoRenderProtocolStatus(reply, ctx);
	reply << "content-type: text/html" << ENDL << ENDL;
	reply << "<html><head>\n";
	reply << "<title>" << errorCode << " " << errorMsg << "</title>\n";
	reply << "</head><body bgcolor=\"silver\">\n";
	reply << "<center>\n";
	reply << "<h1>" << msg << "</h1>\n";
	reply << "Press the back button to return to the previous page!<br><br>\n";
	reply << "<FORM><input type=button value=\"Back\" onClick=\"javascript:history.back(1)\"></FORM>\n";
	reply << "</center>\n";
	ctx.HTMLDebugStores(reply);

	reply << "</body></html>\n";
}
