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

bool HTTPRequestReader::ReadLine(Context &ctx, std::iostream &Ios, String &line, const Anything &clientInfo, bool &hadError)
{
	StartTrace(HTTPRequestReader.ReadLine);
	char c;
	const char eol = '\n';
	line.Trim(0L);
	hadError = false;
	long counter = 0;
	long maxLineSz = ctx.Lookup("LineSizeLimit", 4096L);
	Trace("======================= reading line ==================");
	while ( Ios.get(c).good() /* && !hadError */ ) {
		// read in characterwise
		line.Append(c);
		++counter;
		// error handling
		if ( ((counter > maxLineSz)) && !CheckReqLineSize(ctx, Ios, line.Length(), line, clientInfo)) {
			Trace("Error in read line '" << line << "'");
			hadError = true;
			return false;
		}

		if ( line == ENDL || c == eol ) {
			fRequestBufferSize += counter;
			Trace("fRequestBufferSize:" << fRequestBufferSize << ", maxReqSz:" << ctx.Lookup("RequestSizeLimit", 5120L));
			bool ret = CheckReqBufferSize(ctx, Ios, fRequestBufferSize, line, clientInfo);
			hadError = !ret;
			return ret;
		}
	}
	return false;
}

bool HTTPRequestReader::ReadRequest(Context &ctx, std::iostream &Ios, const Anything &clientInfo)
{
	StartTrace(HTTPRequestReader.ReadRequest);
	// store name of assigned requestprocessor into context to correctly handle error processing
	Anything anyProcessorName = fProc->GetName();
	Context::PushPopEntry<Anything> aRPEntry(ctx, "AssignedRequestProcessorName", anyProcessorName, "RequestProcessor");

	String line(ctx.Lookup("LineSizeLimit", 4096L));
	Trace("======================= reading request ==================");
	bool hadError = false;
	if ( !ReadLine(ctx, Ios, line, clientInfo, hadError) ) return false;
	if ( !HandleFirstLine(ctx, Ios, line, clientInfo) ) return false;
	Trace("First line: [" << line << "]");
	bool ret = true;
	bool doContinue = true;
	while ( doContinue ) {
		doContinue = ReadLine(ctx, Ios, line, clientInfo, hadError);
		Trace("Next line: [" << line << "]");
		if (hadError) {
			return false;
		}
		// end of request
		if (line == ENDL ) {
			break;
		}
		// handle request lines
		fHeader.DoParseHeaderLine(line);
	}
	if ( ctx.Lookup("CheckHeaderFields", 1L) && fHeader.AreSuspiciousHeadersPresent() ) {
		Trace("detected suspicious HTTP headers");
		String tmp;
		OStringStream oss(tmp);
		fHeader.GetInfo().PrintOn(oss, false);
		oss.flush();
		line.Append(" ");
		line.Append(tmp);
		ret = DoHandleError(ctx, Ios, 400, "Possible SSL Renegotiation attack. A header contains a GET/POST request", line, clientInfo, ctx.Lookup("RejectRequestsWithInvalidHeaders", 0L));
	} else if ( fRequestBufferSize == 0 ) {
		ret =  DoHandleError(ctx, Ios, 400, "Empty request", line, clientInfo);
	}
	TraceAny(fHeader.GetInfo(), "RequestHeader:");
	return ret;
}

bool HTTPRequestReader::CheckReqLineSize(Context &ctx, std::iostream &Ios, long lineLength, const String &line, const Anything &clientInfo)
{
	StartTrace(HTTPRequestReader.CheckReqLineSize);
	if (lineLength > ctx.Lookup("LineSizeLimit", 4096L)) {
		String msg;
		msg << "CheckReqLineSize: Request line too Large : [" << lineLength << "] (max " << ctx.Lookup("LineSizeLimit", 4096L) << ")";
		return DoHandleError(ctx, Ios, 413, msg, line, clientInfo);
	}
	return true;
}

bool HTTPRequestReader::CheckReqBufferSize(Context &ctx, std::iostream &Ios, long lineLength, const String &line, const Anything &clientInfo)
{
	StartTrace(HTTPRequestReader.CheckReqBufferSize);
	if (lineLength > ctx.Lookup("RequestSizeLimit", 5120L)) {
		String msg;
		msg << "CheckReqBufferSize: Request too large : [" << lineLength << "] (max " << ctx.Lookup("RequestSizeLimit", 5120L) << ")";
		return DoHandleError(ctx, Ios, 413, msg, line, clientInfo);
	}
	return true;
}

bool HTTPRequestReader::CheckReqURISize(Context &ctx, std::iostream &Ios, long lineLength, const String &line, const Anything &clientInfo)
{
	StartTrace(HTTPRequestReader.CheckReqURISize);
	if (lineLength > ctx.Lookup("URISizeLimit", 512L)) {
		String msg("Request-URI Too Long [");
		msg << lineLength << "], max:" << ctx.Lookup("URISizeLimit", 512L);
		return DoHandleError(ctx, Ios, 414, msg, line, clientInfo);
	}
	return true;
}

bool HTTPRequestReader::HandleFirstLine(Context &ctx, std::iostream &Ios, String &line, const Anything &clientInfo)
{
	StartTrace(HTTPRequestReader.HandleFirstLine);
	long llen = line.Length();
	if (line == ENDL || !(line[long(llen-2)] == '\r' && line[long(llen-1)] == '\n')) {
		return DoHandleError(ctx, Ios, 400, "No request header/body", line, clientInfo);
	} else {
		line.Trim(llen - 2); // cut off ENDL
	}
	if (!CheckReqURISize(ctx, Ios, line.Length(), line, clientInfo)) {
		return false;
	}
	if (!ParseRequest(ctx, Ios, line, clientInfo)) {
		return false;
	}
	return true;
}

bool HTTPRequestReader::ParseRequest(Context &ctx, std::iostream &Ios, String &line, const Anything &clientInfo)
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
			return DoHandleError(ctx, Ios, 405, "Method Not Allowed", line, clientInfo);
		}
	} else {
		return DoHandleError(ctx, Ios, 405, "Method Not Allowed", line, clientInfo);
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
			if (!VerifyUrlPath(ctx, Ios, urlPath, clientInfo)) {
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
					LogError(ctx, 0, reason, cleanUrl, clientInfo, "");
				}
			} else {
				cleanUrl << urlPath;
			}
		} else {
			if (tok == "") {
				return DoHandleError(ctx, Ios, 400, "No URL in request.", line, clientInfo);
			}
			// No arguments supplied
			url = tok;
			Trace("urlPath: " << url);
			if (!VerifyUrlPath(ctx, Ios, url, clientInfo)) {
				return false;
			}
			cleanUrl = url;
		}
		fRequest["REQUEST_URI"] = cleanUrl;
		Trace("Resulting Url " << cleanUrl);
	} else {
		return DoHandleError(ctx, Ios, 405, "Method Not Allowed", line, clientInfo);
	}
	if ( st.NextToken(tok) ) {
		// request protocol
		fRequest["SERVER_PROTOCOL"] = tok;
	} else {
		return DoHandleError(ctx, Ios, 405, "Method Not Allowed", line, clientInfo);
	}
	TraceAny(fRequest, "Request: ParseRequest returning true");
	return true;
}

bool HTTPRequestReader::VerifyUrlPath(Context &ctx, std::iostream &Ios, String &urlPath, const Anything &clientInfo)
{
	StartTrace(HTTPRequestReader.VerifyUrlPath);

	URLUtils::URLCheckStatus eUrlCheckStatus = URLUtils::eOk;
	String urlPathOrig = urlPath;
	// Are all chars which must be URL-encoded really encoded?
	if (URLUtils::CheckUrlEncoding(urlPath, ctx.Lookup("CheckUrlEncodingOverride", "")) == false) {
		return DoHandleError(ctx, Ios, 400, "Not all unsafe chars URL encoded", urlPathOrig, clientInfo);
	}
	if (ctx.Lookup("URLExhaustiveDecode", 0L)) {
		urlPath = URLUtils::ExhaustiveUrlDecode(urlPath, eUrlCheckStatus, false);
	} else {
		urlPath = URLUtils::urlDecode(urlPath, eUrlCheckStatus, false);
	}
	if (eUrlCheckStatus == URLUtils::eSuspiciousChar) {
		// We are done, invalid request
		return DoHandleError(ctx, Ios, 400, "Encoded char above 0x255 detected", urlPathOrig, clientInfo);
	}
	if ( URLUtils::CheckUrlPathContainsUnsafeChars(urlPath, ctx.Lookup("CheckUrlPathContainsUnsafeCharsOverride", ""),
			ctx.Lookup("CheckUrlPathContainsUnsafeCharsAsciiOverride", ""),
			!(ctx.Lookup("CheckUrlPathContainsUnsafeCharsDoNotCheckExtendedAscii", 0L))) ) {
		return DoHandleError(ctx, Ios, 400, "Decoded URL path contains unsafe char", urlPathOrig, clientInfo);
	}
	// "path" part of URL had to be normalized. This may indicate an attack.
	String normalizedUrl =  URLUtils::CleanUpUriPath(urlPath);
	if ( urlPath.Length() !=  normalizedUrl.Length() ) {
		if ( ctx.Lookup("FixDirectoryTraversial", 0L) ) {
			// alter the original url
			urlPathOrig = URLUtils::urlEncode(normalizedUrl, ctx.Lookup("URLEncodeExclude", "/?"));
			LogError(ctx, 0, "Directory traversial attack detected and normalized. Request not rejected because of config settings",
					   urlPathOrig, clientInfo, "");
		} else {
			return DoHandleError(ctx, Ios, 400, "Directory traversial attack", urlPathOrig, clientInfo);
		}
	}
	urlPath = urlPathOrig;

	return true;
}

bool HTTPRequestReader::VerifyUrlArgs(Context &ctx, String &urlArgs)
{
	StartTrace(HTTPRequestReader.VerifyUrlArgs);
	// Are all character which must be URL-encoded really encoded?
	return URLUtils::CheckUrlArgEncoding(urlArgs, ctx.Lookup("CheckUrlArgEncodingOverride", ""));
}

Anything const& HTTPRequestReader::GetRequest()
{
	fRequest["header"] = fHeader.GetInfo();
	return fRequest;
}

// handle error
bool HTTPRequestReader::DoHandleError(Context &ctx, std::iostream &Ios, long errcode, const String &reason, const String &line, const Anything &clientInfo, bool reject, const String &msg)
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
	LogError(ctx, errcode, reason, line, clientInfo, msg);
	// If we do only logging, the request must be valid for further processing to be successful.
	bool ret = !reject;
	Trace("Returning: " << ret);
	return ret;
}

void HTTPRequestReader::LogError(Context &ctx, long errcode, const String &reason, const String &line, const Anything &clientInfo, const String &msg) {
	StartTrace(HTTPRequestReader.LogError);
	Anything request = GetRequest();
	fProc->LogError(ctx, errcode, reason, line, clientInfo, msg, request, "HTTPRequestReader");
}
