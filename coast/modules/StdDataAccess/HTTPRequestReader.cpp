/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPRequestReader.h"

//--- standard modules used ----------------------------------------------------
#include "MIMEHeader.h"
#include "Context.h"
#include "Dbg.h"
#include "AnythingUtils.h"
#include "HTTPProtocolReplyRenderer.h"

//---- HTTPRequestReader -----------------------------------------------------------
HTTPRequestReader::HTTPRequestReader(MIMEHeader &header)
	: fHeader(header), fRequestBufferSize(0)
{
	StartTrace(HTTPRequestReader.HTTPRequestReader);
}

namespace {
	String const strGET("GET", Storage::Global());
	String const strPOST("POST", Storage::Global());
	void PutErrorMessageIntoContext(Context& ctx, long const errorcode, String const& msg, String const& content) {
		StartTrace(HTTPRequestReader.PutErrorMessageIntoContext);
		Anything anyMessage;
		anyMessage["Component"] = "HTTPRequestReader";
		anyMessage["ResponseCode"] = errorcode;
		anyMessage["ResponseMsg"] = HTTPProtocolReplyRenderer::DefaultReasonPhrase(errorcode); //!@FIXME: remove but create and use HTTPResponseMsgRenderer instead where needed, issue #245
		anyMessage["ErrorMessage"] = msg;
		anyMessage["FaultyContent"] = content;
		TraceAny(anyMessage, "generated error message");
		StorePutter::Operate(anyMessage, ctx, "Tmp", ctx.Lookup("RequestProcessorErrorSlot", "HTTPRequestReader.Error"), true);
	}
}

bool HTTPRequestReader::ReadLine(Context &ctx, std::iostream &Ios, long const maxLineSz, String &line)
{
	StartTrace1(HTTPRequestReader.ReadLine, "maxLineSz: " << maxLineSz);
	const char eol = '\n';
	line.Trim(0L);
	if ( Ios.good() ) {
		// read line up to but not including next eol
		line.Append(Ios, maxLineSz, eol);
		Trace("line read [" << line << "] of length: " << line.Length());
		char c = '\0';
		if ( Ios.get(c).good() ) {
			line.Append(c);
		}
		fRequestBufferSize += line.Length();
		if ( line.Length() > maxLineSz ) {
			String msg;
			msg << "Request line too long: >" << line.Length() << ", max: " << maxLineSz << " => check setting of [LineSizeLimit]";
			PutErrorMessageIntoContext(ctx, 413, msg, line);
			return false;
		}
		if ( c != eol) {
			//!@FIXME: should notify that line is not conforming, eg. not ending with ENDL
		}
	}
	// be nice and allow empty lines here - let MIMEHeader decide about validity
	return true;
}

bool HTTPRequestReader::ReadRequest(Context &ctx, std::iostream &Ios)
{
	StartTrace(HTTPRequestReader.ReadRequest);

	long const maxLineSz = ctx.Lookup("LineSizeLimit", 4096L);
	long const maxReqSz = ctx.Lookup("RequestSizeLimit", 5120L);
	String line(maxLineSz);
	Trace("======================= reading request line ==================");
	if ( !ReadLine(ctx, Ios, maxLineSz, line) || RequestSizeLimitExceeded(ctx, maxReqSz, line) ) return false;
	if ( !HandleFirstLine(ctx, line) ) return false;
	Trace("First line: [" << line << "]");
	Trace("======================= reading headers ==================");
	while ( true ) {
		if ( !ReadLine(ctx, Ios, maxLineSz, line) || RequestSizeLimitExceeded(ctx, maxReqSz, line) ) return false;
		Trace("Next line: [" << line << "]");
		if ( not Coast::URLUtils::TrimENDL(line).Length() ) break;
		// handle request lines
		fHeader.DoParseHeaderLine(line);
	}
	if ( fRequestBufferSize == 0 ) {
		PutErrorMessageIntoContext(ctx, 400, "Empty request", line);
		return false;
	}
	TraceAny(fHeader.GetInfo(), "RequestHeader:");
	return true;
}

bool HTTPRequestReader::RequestSizeLimitExceeded(Context &ctx, long const maxReqSz, const String &line) const
{
	StartTrace1(HTTPRequestReader.RequestSizeLimitExceeded, "fRequestBufferSize:" << fRequestBufferSize << ", maxReqSz:" << maxReqSz);
	if (fRequestBufferSize > maxReqSz) {
		String msg;
		msg << "Request too large: >" << fRequestBufferSize << ", max: " << maxReqSz << " => check setting of [RequestSizeLimit]";
		PutErrorMessageIntoContext(ctx, 413, msg, line);
		return true;
	}
	return false;
}

bool HTTPRequestReader::CheckReqURISize(Context &ctx, long lineLength, const String &line) const
{
	StartTrace(HTTPRequestReader.CheckReqURISize);
	long const uriSize = ctx.Lookup("URISizeLimit", 512L);
	if (lineLength > uriSize) {
		String msg;
		msg << "Request-URI too long: >" << lineLength << ", max: " << uriSize << " => check setting of [URISizeLimit]";
		PutErrorMessageIntoContext(ctx, 414, msg, line);
		return false;
	}
	return true;
}

bool HTTPRequestReader::HandleFirstLine(Context &ctx, String &line)
{
	StartTrace(HTTPRequestReader.HandleFirstLine);
	long llen = line.Length();
	if ( not Coast::URLUtils::TrimENDL(line).Length() ) {
		PutErrorMessageIntoContext(ctx, 400, "Empty request line", line);
		return false;
	}
	if ( line.Length() > llen-2L ) {
		PutErrorMessageIntoContext(ctx, 400, "Invalid request line termination", line);
		return false;
	}
	if (!ParseRequest(ctx, line)) {
		return false;
	}
	return true;
}

bool HTTPRequestReader::ParseRequest(Context &ctx, String &line)
{
	StartTrace(HTTPRequestReader.ParseRequest);
	Trace("Line:<" << line << ">");
	Anything anyRequest;
	StringTokenizer st(line, ' ');
	String tok;
	while (st.NextToken(tok)) {
		anyRequest.Append(tok);
	}
	if ( anyRequest.GetSize() != 3L ) {
		PutErrorMessageIntoContext(ctx, 400, "Bad request structure or simple HTTP/0.9 request", line);
		return false;
	}
	tok = anyRequest[0L].AsString().ToUpper();
	if ( strGET.IsEqual(tok) || strPOST.IsEqual(tok) ) { //!@FIXME allow more methods..., make it configurable?
		fRequest["REQUEST_METHOD"] = tok;
	} else {
		PutErrorMessageIntoContext(ctx, 405, "Method Not Allowed", line);
		return false;
	}
	tok = anyRequest[1L].AsString();
	if (!CheckReqURISize(ctx, tok.Length(), line)) {
		return false;
	}
	fRequest["REQUEST_URI"] = tok;
	Trace("Resulting Url [" << tok << "]");
	tok = anyRequest[2L].AsString();
	// request protocol, check for validity
	if ( !tok.StartsWith("HTTP/") ) {
		PutErrorMessageIntoContext(ctx, 400, "server protocol invalid", line);
		return false;
	}
	fRequest["SERVER_PROTOCOL"] = tok;
	TraceAny(fRequest, "Request: ParseRequest returning true");
	return true;
}

Anything const& HTTPRequestReader::GetRequest()
{
	fRequest["header"] = fHeader.GetInfo();
	return fRequest;
}
