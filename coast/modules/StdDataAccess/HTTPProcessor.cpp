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
#include "HTTPRequestReader.h"
#include "HTTPPostRequestBodyParser.h"
#include "HTTPProtocolReplyRenderer.h"
#include "MIMEHeader.h"
#include "Server.h"
#include "AnyIterators.h"
#include "AppLog.h"
#include "AnythingUtils.h"

RegisterRequestProcessor(HTTPProcessor);

// configure request processor with server object
void HTTPProcessor::Init(Server *server) {
	RequestProcessor::Init(server);
}

bool HTTPProcessor::DoReadInput(std::iostream &Ios, Context &ctx)
{
	StartTrace(HTTPProcessor.DoReadInput);
	MethodTimer(HTTPProcessor.DoReadInput, "Reading input", ctx);

	Anything anyProcessorName = GetName();
	Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyProcessorName, "RequestProcessor");

	MIMEHeader header;
	HTTPRequestReader reader(this, header);
	{
		MethodTimer(HTTPRequestReader.ReadRequest, "Reading request header", ctx);
		if (! reader.ReadRequest(ctx, Ios) ) {
			return false;    // this was an error that forbids to process any further
		}
	}
	// GetRequest() returns values in subslot "header"
	Anything request(reader.GetRequest());

	Anything args(ctx.GetRequest());
	args["env"] = request;
	args["query"] = Anything(Anything::ArrayMarker());
	ctx.PushRequest(args);

	// prepare the environment for the framework
	CopyClientInfoIntoRequest(ctx);
	{
		MethodTimer(HTTPRequestReader.ReadRequest, "Reading request body", ctx);
		ReadRequestBody(Ios, request, header, ctx);
	}
	SubTraceAny(request, request, "Arguments:");
	return true;
}

void HTTPProcessor::ReadRequestBody(std::iostream &Ios, Anything &request, MIMEHeader &header, Context &ctx) {
	StartTrace(HTTPProcessor.ReadRequestBody);
	if (request["REQUEST_METHOD"] == "POST") {
		const char *pcSubSlotName = "HTTPPostRequestBodyParser";
		HTTPPostRequestBodyParser sm(header, Ios);
		sm.Parse();
		if (ctx.Lookup("CheckHeaderFields", 1L) && header.AreSuspiciousHeadersPresent()) {
			Anything anyError;
			String strStoreAt("ReadRequestBodyError");
			strStoreAt.Append('.').Append(pcSubSlotName);
			anyError = LogError(ctx, 400,
					"Possible SSL Renegotiation attack. A multipart mime header (in POST) contains a GET/POST request",
					ctx.Lookup("REQUEST_URI", ""), "", pcSubSlotName);
			StorePutter::Operate(anyError, ctx, "Tmp", strStoreAt);
		}
		request["REQUEST_BODY"] = sm.GetContent();
		request["WHOLE_REQUEST_BODY"] = sm.GetUnparsedContent();
		TraceAny(request["REQUEST_BODY"], "Body"); TraceAny(request["WHOLE_REQUEST_BODY"], "Whole Body");
	} else {
		request["header"]["CONTENT-LENGTH"] = 0L;
	}
}

void HTTPProcessor::CopyClientInfoIntoRequest(Context &ctx)
{
	Anything args(ctx.GetRequest());
	long sz = args["ClientInfo"].GetSize();
	for (long i = 0; i < sz; ++i) {
		args["env"]["header"][args["ClientInfo"].SlotName(i)] = args["ClientInfo"][i];
	}
}

bool HTTPProcessor::DoVerifyRequest(Context &ctx) {
	StartTrace(HTTPProcessor.DoVerifyRequest);
	return true;
}

void HTTPProcessor::DoHandleVerifyError(std::ostream &reply, Context &ctx) {
	StartTrace(HTTPProcessor.DoHandleVerifyError);
}

void HTTPProcessor::DoHandleReadInputError(std::ostream &reply, Context &ctx) {
	StartTrace(HTTPProcessor.DoHandleReadInputError);
}

bool HTTPProcessor::DoProcessRequest(std::ostream &reply, Context &ctx)
{
	StartTrace(HTTPProcessor.DoProcessRequest);
	if (IsZipEncodingAcceptedByClient(ctx)) {
		ctx.GetTmpStore()["ClientAcceptsGzipEnc"] = 1L;
	}
	return RequestProcessor::DoProcessRequest(reply, ctx);
}

void HTTPProcessor::DoHandleProcessRequestError(std::ostream &reply, Context &ctx) {
	StartTrace(RequestProcessor.DoHandleReadInputError);
	DoError(reply, "Access denied. Lookuptoken: NDA", ctx);
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
	Trace("Protocol [" << protocol << "] connection [" << connection << "]");
	bool keepAlive = protocol.IsEqual("HTTP/1.1") && connection.ToLower().IsEqual("keep-alive");
	Trace("Keep connection alive: " << keepAlive ? "Yes" : "No");
	return keepAlive;
}

void HTTPProcessor::DoRenderProtocolStatus(std::ostream &os, Context &ctx)
{
	HTTPProtocolReplyRenderer r("HTTPProtocolReplyRenderer");
	r.RenderAll( os, ctx, ROAnything());
}

Anything HTTPProcessor::DoLogError(Context& ctx, long errcode, const String &reason, const String &line, const String &msg, const char *who)
{
	StartTrace(HTTPProcessor.DoLogError);
	// define SecurityLog according to the AppLog rules if you want to see this output.
	Anything tmp;
	tmp["REMOTE_ADDR"] = ctx.Lookup("ClientInfo.REMOTE_ADDR", "");
	tmp["HTTPS"] = ctx.Lookup("ClientInfo.HTTPS", "");
	tmp["Request"] = ctx.GetRequest();
	tmp["HttpStatusCode"]	=  errcode;
	tmp["HttpResponseMsg"] = msg;
	tmp["Reason"] = reason;
	tmp["FaultyRequestLine"] = line;
	StorePutter::Operate(tmp, ctx, "Tmp", who);
	AppLogModule::Log(ctx, "SecurityLog", AppLogModule::eERROR);
	return tmp;
}

//! render the protocol specific error msg
void HTTPProcessor::DoError(std::ostream &reply, const String &msg, Context &ctx)
{
	StartTrace1(HTTPProcessor.DoError, "message [" << msg << "]");
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
