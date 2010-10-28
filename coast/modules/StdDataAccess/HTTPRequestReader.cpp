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
#include "HTTPProcessor.h"
#include "MIMEHeader.h"
#include "Context.h"
#include "Dbg.h"
#include "StringStream.h"
#include "AnythingUtils.h"

//---- HTTPRequestReader -----------------------------------------------------------
HTTPRequestReader::HTTPRequestReader(RequestProcessor *p, MIMEHeader &header)
	: fProc(p), fHeader(header), fRequestBufferSize(0), fFirstLine(true)
{
	StartTrace(HTTPRequestReader.HTTPRequestReader);
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
			msg << "LineSizeLimitExceeded: Request line too Large : [" << line.Length() << "] (max " << maxLineSz << ")";
			Trace(msg);
			DoHandleError(ctx, Ios, 413, msg, line);
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
	// store name of assigned requestprocessor into context to correctly handle error processing
	Anything anyProcessorName = fProc->GetName();
	Context::PushPopEntry<Anything> aRPEntry(ctx, "AssignedRequestProcessorName", anyProcessorName, "RequestProcessor");

	long const maxLineSz = ctx.Lookup("LineSizeLimit", 4096L);
	long const maxReqSz = ctx.Lookup("RequestSizeLimit", 5120L);
	String line(maxLineSz);
	Trace("======================= reading request line ==================");
	if ( !ReadLine(ctx, Ios, maxLineSz, line) || RequestSizeLimitExceeded(ctx, Ios, maxReqSz, line) ) return false;
	if ( !HandleFirstLine(ctx, Ios, line) ) return false;
	Trace("First line: [" << line << "]");
	Trace("======================= reading headers ==================");
	bool ret = true;
	while ( true ) {
		if ( !ReadLine(ctx, Ios, maxLineSz, line) || RequestSizeLimitExceeded(ctx, Ios, maxReqSz, line) ) return false;
		Trace("Next line: [" << line << "]");
		if ( line.Length() == 0L || line.IsEqual(ENDL) ) {
			// end of request
			break;
		}
		// handle request lines
		fHeader.DoParseHeaderLine(line);
	}
	if ( fRequestBufferSize == 0 ) {
		DoHandleError(ctx, Ios, 400, "Empty request", line);
		return false;
	}
	if ( ctx.Lookup("CheckHeaderFields", 1L) && fHeader.AreSuspiciousHeadersPresent() ) {
		Trace("detected suspicious HTTP headers");
		String tmp;
		OStringStream oss(tmp);
		fHeader.GetInfo().PrintOn(oss, false);
		oss.flush();
		line.Append(" ");
		line.Append(tmp);
		ret = DoHandleError(ctx, Ios, 400, "Possible SSL Renegotiation attack. A header contains a GET/POST request", line, ctx.Lookup("RejectRequestsWithInvalidHeaders", 0L));
	}
	TraceAny(fHeader.GetInfo(), "RequestHeader:");
	return ret;
}

bool HTTPRequestReader::RequestSizeLimitExceeded(Context &ctx, std::iostream &Ios, long const maxReqSz, const String &line)
{
	StartTrace1(HTTPRequestReader.RequestSizeLimitExceeded, "fRequestBufferSize:" << fRequestBufferSize << ", maxReqSz:" << maxReqSz);
	if (fRequestBufferSize > maxReqSz) {
		String msg;
		msg << "RequestSizeLimitExceeded: Request too large : [" << fRequestBufferSize << "] (max " << maxReqSz << ")";
		Trace(msg);
		DoHandleError(ctx, Ios, 413, msg, line);
		return true;
	}
	return false;
}

bool HTTPRequestReader::CheckReqURISize(Context &ctx, std::iostream &Ios, long lineLength, const String &line)
{
	StartTrace(HTTPRequestReader.CheckReqURISize);
	if (lineLength > ctx.Lookup("URISizeLimit", 512L)) {
		String msg("Request-URI Too Long [");
		msg << lineLength << "], max:" << ctx.Lookup("URISizeLimit", 512L);
		return DoHandleError(ctx, Ios, 414, msg, line);
	}
	return true;
}

bool HTTPRequestReader::HandleFirstLine(Context &ctx, std::iostream &Ios, String &line)
{
	StartTrace(HTTPRequestReader.HandleFirstLine);
	long llen = line.Length();
	if (line == ENDL || !(line[long(llen-2)] == '\r' && line[long(llen-1)] == '\n')) {
		return DoHandleError(ctx, Ios, 400, "No request header/body", line);
	} else {
		line.Trim(llen - 2); // cut off ENDL
	}
	if (!CheckReqURISize(ctx, Ios, line.Length(), line)) {
		return false;
	}
	if (!ParseRequest(ctx, Ios, line)) {
		return false;
	}
	return true;
}

bool HTTPRequestReader::ParseRequest(Context &ctx, std::iostream &Ios, String &line)
{
	StartTrace(HTTPRequestReader.ParseRequest);
	Trace("Line:<" << line << ">");
	StringTokenizer st(line, ' ');
	String tok;

	if ( st.NextToken(tok) ) {
		// request type
		tok.ToUpper();
		if ( tok == "GET" || tok == "POST" ) { //!@FIXME allow more methods..., make it configurable?
			fRequest["REQUEST_METHOD"] = tok;
		} else {
			return DoHandleError(ctx, Ios, 405, "Method Not Allowed", line);
		}
	} else {
		return DoHandleError(ctx, Ios, 405, "Method Not Allowed", line);
	}
	String cleanUrl;
	String url;
	if ( st.NextToken(tok) ) {
		// extract request url path, without trailing ? or;

		long argDelimPos = tok.FirstCharOf("?;");
		if (argDelimPos != -1) {
			char argDelim = tok[argDelimPos];
			String urlPath(tok.SubString(0, argDelimPos));
			Trace("UrlPath: argDelimPos: " << argDelimPos << " argDelim: " << argDelim << "urlPath: " << urlPath);
			if (!VerifyUrlPath(ctx, Ios, urlPath)) {
				return false;
			}
			String urlArgs;
			// Do we have any args in original, unchanged request?
			bool hasArgs(argDelimPos != tok.Length() - 1);
			if ( hasArgs ) {
				urlArgs = tok.SubString(argDelimPos + 1, tok.Length());
				Trace("UrlArgs: argDelimPos: " << argDelimPos << " urlArgs: " << urlArgs);
				cleanUrl << urlPath << argDelim <<  urlArgs;
				if ( !VerifyUrlArgs(ctx, urlArgs) ) {
					String reason("Argument string (after ");
					reason << argDelim << ") was not correctly encoded. Request not rejected.";
					LogError(ctx, 0, reason, cleanUrl, "");
				}
			} else {
				cleanUrl << urlPath;
			}
		} else {
			if (tok == "") {
				return DoHandleError(ctx, Ios, 400, "No URL in request.", line);
			}
			// No arguments supplied
			url = tok;
			Trace("urlPath: " << url);
			if (!VerifyUrlPath(ctx, Ios, url)) {
				return false;
			}
			cleanUrl = url;
		}
		fRequest["REQUEST_URI"] = cleanUrl;
		Trace("Resulting Url " << cleanUrl);
	} else {
		return DoHandleError(ctx, Ios, 405, "Method Not Allowed", line);
	}
	if ( st.NextToken(tok) ) {
		// request protocol
		fRequest["SERVER_PROTOCOL"] = tok;
	} else {
		return DoHandleError(ctx, Ios, 405, "Method Not Allowed", line);
	}
	TraceAny(fRequest, "Request: ParseRequest returning true");
	return true;
}

bool HTTPRequestReader::VerifyUrlPath(Context &ctx, std::iostream &Ios, String &urlPath)
{
	StartTrace(HTTPRequestReader.VerifyUrlPath);

	Coast::URLUtils::URLCheckStatus eUrlCheckStatus = Coast::URLUtils::eOk;
	String urlPathOrig = urlPath;
	// Are all chars which must be URL-encoded really encoded?
	if (Coast::URLUtils::CheckUrlEncoding(urlPath, ctx.Lookup("CheckUrlEncodingOverride", "")) == false) {
		return DoHandleError(ctx, Ios, 400, "Not all unsafe chars URL encoded", urlPathOrig);
	}
	if (ctx.Lookup("URLExhaustiveDecode", 0L)) {
		urlPath = Coast::URLUtils::ExhaustiveUrlDecode(urlPath, eUrlCheckStatus, false);
	} else {
		urlPath = Coast::URLUtils::urlDecode(urlPath, eUrlCheckStatus, false);
	}
	if (eUrlCheckStatus == Coast::URLUtils::eSuspiciousChar) {
		// We are done, invalid request
		return DoHandleError(ctx, Ios, 400, "Encoded char above 0x255 detected", urlPathOrig);
	}
	if ( Coast::URLUtils::CheckUrlPathContainsUnsafeChars(urlPath, ctx.Lookup("CheckUrlPathContainsUnsafeCharsOverride", ""),
			ctx.Lookup("CheckUrlPathContainsUnsafeCharsAsciiOverride", ""),
			!(ctx.Lookup("CheckUrlPathContainsUnsafeCharsDoNotCheckExtendedAscii", 0L))) ) {
		return DoHandleError(ctx, Ios, 400, "Decoded URL path contains unsafe char", urlPathOrig);
	}
	// "path" part of URL had to be normalized. This may indicate an attack.
	String normalizedUrl =  Coast::URLUtils::CleanUpUriPath(urlPath);
	if ( urlPath.Length() !=  normalizedUrl.Length() ) {
		if ( ctx.Lookup("FixDirectoryTraversial", 0L) ) {
			// alter the original url
			urlPathOrig = Coast::URLUtils::urlEncode(normalizedUrl, ctx.Lookup("URLEncodeExclude", "/?"));
			LogError(ctx, 0, "Directory traversial attack detected and normalized. Request not rejected because of config settings",
					   urlPathOrig, "");
		} else {
			return DoHandleError(ctx, Ios, 400, "Directory traversial attack", urlPathOrig);
		}
	}
	urlPath = urlPathOrig;

	return true;
}

bool HTTPRequestReader::VerifyUrlArgs(Context &ctx, String &urlArgs)
{
	StartTrace(HTTPRequestReader.VerifyUrlArgs);
	// Are all character which must be URL-encoded really encoded?
	return Coast::URLUtils::CheckUrlArgEncoding(urlArgs, ctx.Lookup("CheckUrlArgEncodingOverride", ""));
}

Anything const& HTTPRequestReader::GetRequest()
{
	fRequest["header"] = fHeader.GetInfo();
	return fRequest;
}

// handle error
bool HTTPRequestReader::DoHandleError(Context &ctx, std::iostream &Ios, long errcode, const String &reason, const String &line, bool reject, const String &msg)
{
	StartTrace(HTTPRequestReader.DoHandleError);
	Trace("Errcode: [" << errcode << "] Message: [" << msg << "] Faulty line: [" << line << "] reject: [" << reject << "]");
	if ( reject ) {
		if ( !!Ios ) {
			Anything anyErrCode = errcode;
			StorePutter::Operate(anyErrCode, ctx, "Tmp", "HTTPStatus.ResponseCode");
			fProc->Error(Ios, msg, ctx);
			Ios << ENDL;
			Ios.flush();
			Ios.clear(std::ios::badbit);
		} else {
			Trace("Can not send error msg.");
		}
	}
	LogError(ctx, errcode, reason, line, msg);
	// If we do only logging, the request must be valid for further processing to be successful.
	bool ret = !reject;
	Trace("Returning: " << ret);
	return ret;
}

void HTTPRequestReader::LogError(Context &ctx, long errcode, const String &reason, const String &line, const String &msg) {
	StartTrace(HTTPRequestReader.LogError);
	fProc->LogError(ctx, errcode, reason, line, msg, "HTTPRequestReader");
}
