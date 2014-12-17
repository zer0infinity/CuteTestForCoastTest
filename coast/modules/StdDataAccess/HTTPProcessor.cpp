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
	void CopyClientInfoIntoRequest(Context &ctx) {
		Anything args(ctx.GetRequest());
		long sz = args["ClientInfo"].GetSize();
		for (long i = 0; i < sz; ++i) {
			args["env"]["header"][args["ClientInfo"].SlotName(i)] = args["ClientInfo"][i];
		}
	}
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
		Anything GenerateErrorMessageAny(Context& ctx, long const errorcode, String const& msg, String const& content, String const& component) {
			StartTrace(HTTPProcessor.GenerateErrorMessageAny);
			Anything anyMessage;
			anyMessage["Component"] = component;
			anyMessage[coast::http::constants::protocolCodeSlotname] = errorcode;
			anyMessage[coast::http::constants::protocolMsgSlotname] = HTTPProtocolReplyRenderer::DefaultReasonPhrase(errorcode); //!@FIXME: remove but create and use HTTPResponseMsgRenderer instead where needed, issue #245
			anyMessage["ErrorMessage"] = msg;
			anyMessage["FaultyContent"] = content;
			TraceAny(anyMessage, "generated error message");
			return anyMessage;
		}
		void PutErrorMessageIntoContext(Context& ctx, long const errorcode, String const& msg, String const& content, String const& component) {
			StartTrace(HTTPProcessor.PutErrorMessageIntoContext);
			Anything anyMessage = GenerateErrorMessageAny(ctx, errorcode, msg, content, component);
			StorePutter::Operate(anyMessage, ctx, "Tmp", ctx.Lookup("RequestProcessorErrorSlot", "HTTPProcessor.Error"), true);
		}
	}
}

// configure request processor with server object
void HTTPProcessor::Init(Server *server) {
	RequestProcessor::Init(server);
}

MIMEHeader HTTPProcessor::GetMIMEHeader() const {
	return MIMEHeader();
}

HTTPRequestReader HTTPProcessor::GetRequestReader(MIMEHeader& header) const {
	return HTTPRequestReader(header);
}

HTTPPostRequestBodyParser HTTPProcessor::GetRequestBodyParser(MIMEHeader& header) const {
	return HTTPPostRequestBodyParser(header);
}

bool HTTPProcessor::DoReadRequestHeader(std::iostream &Ios, Context &ctx, HTTPRequestReader &reader) {
	StartTrace(HTTPProcessor.DoReadRequestHeader);
	MethodTimer(HTTPProcessor.DoReadRequestHeader, "Reading request header", ctx);
	return reader.ReadRequest(ctx, Ios);
}

bool HTTPProcessor::DoPrepareContextRequest(std::iostream &Ios, Context &ctx, Anything &request, HTTPRequestReader &reader) {
	StartTrace(HTTPProcessor.DoPrepareContextRequest);
	MethodTimer(HTTPProcessor.DoPrepareContextRequest, "Preparing request for context", ctx);
	Anything args(ctx.GetRequest());
	args["env"] = request;
	args["query"] = Anything(Anything::ArrayMarker());
	ctx.PushRequest(args);
	// prepare the environment for the framework
	CopyClientInfoIntoRequest(ctx);
	return true;
}

bool HTTPProcessor::DoReadRequestBody(std::iostream &Ios, Context &ctx, Anything &request, HTTPRequestReader &reader) {
	StartTrace(HTTPProcessor.DoReadRequestBody);
	MethodTimer(HTTPProcessor.DoReadRequestBody, "Reading request body", ctx);
	if ( request["REQUEST_METHOD"].AsString().IsEqual(coast::http::constants::postMethodSlotname) ) {
		HTTPPostRequestBodyParser sm = GetRequestBodyParser(reader.GetHeaderParser());
		try {
			sm.Parse(Ios);
		} catch (MIMEHeader::LineSizeExceededException &e) {
			coast::http::PutErrorMessageIntoContext(ctx, 413, String(e.what()).Append(" => check setting of [LineSizeLimit]"), e.fLine, "HTTPProcessor::DoReadRequestBody");
		} catch (MIMEHeader::RequestSizeExceededException &e) {
			coast::http::PutErrorMessageIntoContext(ctx, 413, String(e.what()).Append(" => check setting of [RequestSizeLimit]"), e.fLine, "HTTPProcessor::DoReadRequestBody");
		} catch (MIMEHeader::InvalidLineException &e) {
			coast::http::PutErrorMessageIntoContext(ctx, 400, e.what(), e.fLine, "HTTPProcessor::DoReadRequestBody");
		}
		request["REQUEST_BODY"] = sm.GetContent();
		TraceAny(request["REQUEST_BODY"], "Body");
		request["WHOLE_REQUEST_BODY"] = sm.GetUnparsedContent();
		TraceAny(request["WHOLE_REQUEST_BODY"], "Whole Body");
	}
	return true;
}

bool HTTPProcessor::DoReadInput(std::iostream &Ios, Context &ctx)
{
	StartTrace(HTTPProcessor.DoReadInput);
	MethodTimer(HTTPProcessor.DoReadInput, "Reading input", ctx);

	MIMEHeader header = GetMIMEHeader();
	HTTPRequestReader reader = GetRequestReader(header);
	if ( not DoReadRequestHeader(Ios, ctx, reader) ) {
		return false;
	}

	// GetRequest() returns headers from request in subslot "header"
	Anything request(reader.GetRequest());
	if ( not DoPrepareContextRequest(Ios, ctx, request, reader) ) {
		return false;
	}

	if ( not DoReadRequestBody(Ios, ctx, request, reader) ) {
		return false;
	}
	SubTraceAny(request, request, "Arguments:");
	return true;
}

bool HTTPProcessor::DoVerifyRequest(Context &ctx) {
	StartTrace(HTTPProcessor.DoVerifyRequest);
	return true;
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
		if (TriggerEnabled(HTTPProcessor.GenericRequestProcessorErrorHandler)) {
			OStringStream ostr;
			ctx.DebugStores("Stores after error", ostr, true);
			Trace(ostr.str());
		}
		ROAnything roaErrorMessages;
		if (ctx.Lookup(ctx.Lookup("RequestProcessorErrorSlot", "NonExistingSlotname"), roaErrorMessages)) {
			LogError(ctx);
			//!@FIXME: maybe we should log all of them?
			Anything anyErrCode = roaErrorMessages[0L][coast::http::constants::protocolCodeSlotname].DeepClone();
			StorePutter::Operate(anyErrCode, ctx, "Tmp", String("HTTPStatus.").Append(coast::http::constants::protocolCodeSlotname));
			ErrorReply(reply, roaErrorMessages[0L]["ErrorMessage"].AsString(), ctx);
		}
	}
}

bool HTTPProcessor::DoHandleReadInputError(std::iostream &Ios, Context &ctx) {
	StartTrace(HTTPProcessor.DoHandleReadInputError);
	GenericRequestProcessorErrorHandler(Ios, ctx);
	return false;
}

bool HTTPProcessor::DoHandleVerifyRequestError(std::iostream &Ios, Context &ctx) {
	StartTrace(HTTPProcessor.DoHandleVerifyRequestError);
	GenericRequestProcessorErrorHandler(Ios, ctx);
	return false;
}

bool HTTPProcessor::DoHandleProcessRequestError(std::iostream &Ios, Context &ctx) {
	StartTrace(HTTPProcessor.DoHandleProcessRequestError);
	GenericRequestProcessorErrorHandler(Ios, ctx);
	return false;
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
