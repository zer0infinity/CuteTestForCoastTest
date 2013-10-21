/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPProcessor.h"
#include "Timers.h"
#include "HTTPRequestReader.h"
#include "HTTPPostRequestBodyParser.h"
#include "HTTPProtocolReplyRenderer.h"
#include "MIMEHeader.h"
#include "AnyIterators.h"
#include "AppLog.h"
#include "AnythingUtils.h"
#include "StringStream.h"
#include "HTTPConstants.h"

RegisterRequestProcessor(HTTPProcessor);

namespace {
	String const strGET("GET", coast::storage::Global());
	String const strPOST("POST", coast::storage::Global());

	void PutErrorMessageIntoContext(Context& ctx, long const errorcode, String const& msg, String const& content) {
		StartTrace(HTTPProcessor.PutErrorMessageIntoContext);
		Anything anyMessage;
		anyMessage[coast::http::constants::protocolCodeSlotname] = errorcode;
		anyMessage[coast::http::constants::protocolMsgSlotname] = HTTPProtocolReplyRenderer::DefaultReasonPhrase(errorcode); //!@FIXME: remove but create and use HTTPResponseMsgRenderer instead where needed, issue #245
		anyMessage["ErrorMessage"] = msg;
		anyMessage["FaultyContent"] = content;
		TraceAny(anyMessage, "generated error message");
		StorePutter::Operate(anyMessage, ctx, "Tmp", ctx.Lookup("RequestProcessorErrorSlot", "HTTPProcessor.Error"), true);
	}
}

// configure request processor with server object
void HTTPProcessor::Init(Server *server) {
	RequestProcessor::Init(server);
}

bool HTTPProcessor::DoReadInput(std::iostream &Ios, Context &ctx)
{
	StartTrace(HTTPProcessor.DoReadInput);
	MethodTimer(HTTPProcessor.DoReadInput, "Reading input", ctx);

	MIMEHeader header;
	HTTPRequestReader reader(header);
	{
		MethodTimer(HTTPRequestReader.ReadRequest, "Reading request header", ctx);
		if ( not reader.ReadRequest(ctx, Ios) ) {
			return false;
		}
	}
	// GetRequest() returns values in subslot "header"
	Anything request(reader.GetRequest());

	Anything args(ctx.GetRequest());	//!@FIXME: replace with reference
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
	if ( strPOST.IsEqual(request["REQUEST_METHOD"].AsCharPtr()) ) {
		HTTPPostRequestBodyParser sm(header);
		try {
			sm.Parse(Ios);
		} catch (MIMEHeader::LineSizeExceededException &e) {
			PutErrorMessageIntoContext(ctx, 413, String(e.what()).Append(" => check setting of [LineSizeLimit]"), e.fLine);
		} catch (MIMEHeader::HeaderSizeExceededException &e) {
			PutErrorMessageIntoContext(ctx, 413, String(e.what()).Append(" => check setting of [RequestSizeLimit]"), e.fLine);
		} catch (MIMEHeader::InvalidLineException &e) {
			PutErrorMessageIntoContext(ctx, 400, e.what(), e.fLine);
		}
		request["REQUEST_BODY"] = sm.GetContent();
		TraceAny(request["REQUEST_BODY"], "Body");
		request["WHOLE_REQUEST_BODY"] = sm.GetUnparsedContent();
		TraceAny(request["WHOLE_REQUEST_BODY"], "Whole Body");
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

namespace coast {
	namespace http {
		void RenderHTTPProtocolStatus(std::ostream &os, Context &ctx) {
			StartTrace(HTTPProcessor.RenderHTTPProtocolStatus);
			Anything defaultSpec;
			defaultSpec["HTTPProtocolReplyRenderer"] = Anything();
			ROAnything protocolRendererSpec;
			Renderer::Render(os, ctx, ctx.Lookup("ProtocolReplyRenderer", protocolRendererSpec) ? protocolRendererSpec : defaultSpec);
		}
	}
}

namespace {
	void ErrorReply(std::ostream &reply, const String &msg, Context &ctx) {
		StartTrace1(HTTPProcessor.ErrorReply, "message [" << msg << "]");
		long errorCode = ctx.Lookup(String("HTTPStatus.").Append(coast::http::constants::protocolCodeSlotname), 400L);
		String errorMsg(HTTPProtocolReplyRenderer::DefaultReasonPhrase(errorCode));
		coast::http::RenderHTTPProtocolStatus(reply, ctx);
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
		reply << std::flush;
	}

	void LogError(Context& ctx) {
		StartTrace(HTTPProcessor.LogError);
		AppLogModule::Log(ctx, "SecurityLog", AppLogModule::eERROR);
	}

	void GenericRequestProcessorErrorHandler(std::ostream &reply, Context &ctx) {
		StartTrace(HTTPProcessor.GenericRequestProcessorErrorHandler);
		OStringStream ostr;
		ctx.DebugStores("Stores after error", ostr, true);
		Trace(ostr.str());
		ROAnything roaErrorMessages;
		if ( ctx.Lookup(ctx.Lookup("RequestProcessorErrorSlot","NonExistingSlotname"), roaErrorMessages) ) {
			LogError(ctx);
			//!@FIXME: maybe we should log all of them?
			Anything anyErrCode = roaErrorMessages[0L][coast::http::constants::protocolCodeSlotname].DeepClone();
			StorePutter::Operate(anyErrCode, ctx, "Tmp", String("HTTPStatus.").Append(coast::http::constants::protocolCodeSlotname));
			ErrorReply(reply, roaErrorMessages[0L]["ErrorMessage"].AsString(), ctx);
		}
	}
}

void HTTPProcessor::DoHandleReadInputError(std::ostream &reply, Context &ctx) {
	StartTrace(HTTPProcessor.DoHandleReadInputError);
	GenericRequestProcessorErrorHandler(reply, ctx);
}

void HTTPProcessor::DoHandleVerifyError(std::ostream &reply, Context &ctx) {
	StartTrace(HTTPProcessor.DoHandleVerifyError);
	GenericRequestProcessorErrorHandler(reply, ctx);
}

void HTTPProcessor::DoHandleProcessRequestError(std::ostream &reply, Context &ctx) {
	StartTrace(HTTPProcessor.DoHandleProcessRequestError);
	GenericRequestProcessorErrorHandler(reply, ctx);
}

bool HTTPProcessor::DoProcessRequest(std::ostream &reply, Context &ctx)
{
	StartTrace(HTTPProcessor.DoProcessRequest);
	if (IsZipEncodingAcceptedByClient(ctx)) {
		ctx.GetTmpStore()["ClientAcceptsGzipEnc"] = 1L;
	}
	return RequestProcessor::DoProcessRequest(reply, ctx);
}

void HTTPProcessor::DoRenderProtocolStatus(std::ostream &os, Context &ctx) {
	coast::http::RenderHTTPProtocolStatus(os, ctx);
}

namespace {
	bool containsLowercaseValue(ROAnything roaContainer, String const &strValue) {
		AnyExtensions::LeafIterator<ROAnything> iter(roaContainer);
		ROAnything roaCurrAny;
		while (iter.Next(roaCurrAny)) {
			if (roaCurrAny.AsString().ToLower().Contains(strValue) != -1 ) {
				return true;
			}
		}
		return false;
	}
}

bool HTTPProcessor::IsZipEncodingAcceptedByClient(Context &ctx)
{
	StartTrace(HTTPProcessor.IsZipEncodingAcceptedByClient);
	TraceAny(ctx.GetRequest(), "Request");
	ROAnything roaEncoding;
	if (!ctx.Lookup("DisableZipEncoding", 0L) && ctx.Lookup("header.ACCEPT-ENCODING", roaEncoding) ) {
		TraceAny(roaEncoding, "accepted encodings");
		return containsLowercaseValue(roaEncoding, "gzip");
	}
	return false;
}

bool HTTPProcessor::DoKeepConnectionAlive(Context &ctx)
{
	StartTrace(HTTPProcessor.DoKeepConnectionAlive);
	String protocol = ctx.Lookup("SERVER_PROTOCOL", "");
	ROAnything roaConnection = ctx.Lookup("header.CONNECTION");
	TraceAny(roaConnection, "Protocol [" << protocol << "]");
	bool keepAlive = protocol.IsEqual("HTTP/1.1") && containsLowercaseValue(roaConnection, "keep-alive");
	Trace("Keep connection alive: " << keepAlive ? "Yes" : "No");
	return keepAlive;
}
