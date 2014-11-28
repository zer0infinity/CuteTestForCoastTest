/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPRequestReader.h"
#include "MIMEHeader.h"
#include "AnythingUtils.h"
#include "HTTPProtocolReplyRenderer.h"
#include "HTTPConstants.h"

namespace {
	String const strGET("GET", coast::storage::Global());
	String const strPOST("POST", coast::storage::Global());
	void PutErrorMessageIntoContext(Context& ctx, long const errorcode, String const& msg, String const& content) {
		StartTrace(HTTPRequestReader.PutErrorMessageIntoContext);
		Anything anyMessage;
		anyMessage["Component"] = "HTTPRequestReader";
		anyMessage[coast::http::constants::protocolCodeSlotname] = errorcode;
		anyMessage[coast::http::constants::protocolMsgSlotname] = HTTPProtocolReplyRenderer::DefaultReasonPhrase(errorcode); //!@FIXME: remove but create and use HTTPResponseMsgRenderer instead where needed, issue #245
		anyMessage["ErrorMessage"] = msg;
		anyMessage["FaultyContent"] = content;
		TraceAny(anyMessage, "generated error message");
		StorePutter::Operate(anyMessage, ctx, "Tmp", ctx.Lookup("RequestProcessorErrorSlot", "HTTPRequestReader.Error"), true);
	}
	struct URISizeExceededException : MIMEHeader::SizeExceededException {
		URISizeExceededException(String const& msg, String const& line, long lMaxSize, long lActualSize) : MIMEHeader::SizeExceededException(msg, line, lMaxSize, lActualSize) {}
	};
	struct InvalidLineWithCodeException : MIMEHeader::InvalidLineException {
		long fCode;
		InvalidLineWithCodeException(String const& msg, String const& line, long code) : MIMEHeader::InvalidLineException(msg, line), fCode(code) {}
	};

	void ParseRequestLine(Context &ctx, Anything &request, String &line) {
		StartTrace(HTTPRequestReader.ParseRequestLine); Trace("Line:<" << line << ">");
		Anything anyRequest;
		coast::urlutils::Split(line, ' ', anyRequest, ' ');
		if (anyRequest.GetSize() != 3L)
			throw MIMEHeader::InvalidLineException("Bad request structure or simple HTTP/0.9 request", line);
		String tok = anyRequest[0L].AsString().ToUpper();
		if (strGET.IsEqual(tok) || strPOST.IsEqual(tok)) { //!@FIXME allow more methods..., make it configurable?
			request["REQUEST_METHOD"] = tok;
		} else {
			throw InvalidLineWithCodeException("Method Not Allowed", line, 405);
		}
		tok = anyRequest[1L].AsString();
		long const uriSize = ctx.Lookup("URISizeLimit", 512L);
		if (tok.Length() > uriSize) {
			throw URISizeExceededException("Request-URI too long", line, uriSize, tok.Length());
		}
		request["REQUEST_URI"] = tok;
		Trace("Resulting Url [" << tok << "]");
		tok = anyRequest[2L].AsString();
		// request protocol, check for validity
		if (!tok.StartsWith("HTTP/")) {
			throw InvalidLineWithCodeException("Invalid server protocol", line, 400);
		}
		request["SERVER_PROTOCOL"] = tok;
		TraceAny(request, "Request: ParseRequest returning true");
	}

	void HandleFirstLine(Context &ctx, Anything &request, String &line) {
		StartTrace(HTTPRequestReader.HandleFirstLine);
		long llen = line.Length();
		if (not coast::urlutils::TrimENDL(line).Length()) {
			throw MIMEHeader::InvalidLineException("Empty request line", line);
		}
		if (line.Length() > llen - 2L) {
			throw MIMEHeader::InvalidLineException("Invalid request line termination", line);
		}
		ParseRequestLine(ctx, request, line);
	}
}

bool HTTPRequestReader::ReadRequest(Context &ctx, std::iostream &Ios) {
	StartTrace(HTTPRequestReader.ReadRequest);
	return DoReadRequest(ctx, Ios);
}

bool HTTPRequestReader::DoReadRequest(Context &ctx, std::iostream &Ios) {
	StartTrace(HTTPRequestReader.DoReadRequest);
	long const maxLineSz = ctx.Lookup("LineSizeLimit", 4096L);
	long const maxReqSz = ctx.Lookup("RequestSizeLimit", 5120L);
	String line(maxLineSz);
	try {
		coast::streamutils::getLineFromStream(Ios, line, maxLineSz);
		fRequestBufferSize += line.Length();
		if (fRequestBufferSize > maxReqSz) {
			throw MIMEHeader::RequestSizeExceededException("RequestSizeLimit exceeded", line, maxReqSz, fRequestBufferSize);
		}
		HandleFirstLine(ctx, fRequest, line);
		fHeader.ParseHeaders(Ios, maxLineSz, maxReqSz-fRequestBufferSize);
		fRequestBufferSize += fHeader.GetParsedHeaderLength();
		if (fRequestBufferSize == 0) {
			throw InvalidLineWithCodeException("Empty request", line, 400);
		}
		TraceAny(fHeader.GetHeaderInfo(), "RequestHeader:");
		return true;
	} catch (MIMEHeader::LineSizeExceededException &e) {
		PutErrorMessageIntoContext(ctx, 413, String(e.what()).Append(" => check setting of [LineSizeLimit]"), e.fLine);
	} catch (MIMEHeader::RequestSizeExceededException &e) {
		PutErrorMessageIntoContext(ctx, 413, String(e.what()).Append(" => check setting of [RequestSizeLimit]"), e.fLine);
	} catch (URISizeExceededException &e) {
		PutErrorMessageIntoContext(ctx, 414, String(e.what()).Append(" => check setting of [URISizeLimit]"), e.fLine);
	} catch (InvalidLineWithCodeException &e) {
		PutErrorMessageIntoContext(ctx, e.fCode, e.what(), e.fLine);
	} catch (MIMEHeader::InvalidLineException &e) {
		PutErrorMessageIntoContext(ctx, 400, e.what(), e.fLine);
	} catch (MIMEHeader::StreamNotGoodException &e) {
		PutErrorMessageIntoContext(ctx, 400, e.what(), "");
	}
	return false;
}

Anything const& HTTPRequestReader::GetRequest()
{
	fRequest["header"] = fHeader.GetHeaderInfo();
	return fRequest;
}
