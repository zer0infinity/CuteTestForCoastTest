/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RequestReader.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "AppLog.h"

//--- c-library modules used ---------------------------------------------------

//---- RequestReader -----------------------------------------------------------
RequestReader::RequestReader(HTTPProcessor *p, MIMEHeader &header)
	: fProc(p), fHeader(header), fRequestBufferSize(0), fFirstLine(true)
{
	StartTrace(RequestReader.RequestReader);
}

bool RequestReader::ReadLine(iostream &Ios, String &line, const Anything &clientInfo)
{
	StartTrace(RequestReader.ReadLine);
	char c;
	const char eol = '\n';
	line = "";
	long counter = 0;
	long maxLineSz = fProc->fLineSizeLimit;
	Trace("======================= reading input ==================");

	while ( Ios.get(c).good() ) {
		// read in characterwise
		line << c;
		counter++;
		// error handling
		if ( ((counter > maxLineSz)) && !CheckReqLineSize(Ios, line.Length(), line, clientInfo)) {
			Trace("Error in read line '" << line << "'");
			return false;
		}

		if ( line == ENDL || c == eol ) {
#ifdef REQ_TRACING
			fRequestBuffer << line;
#endif
			fRequestBufferSize += counter;
			Trace("fRequestBufferSize:" << fRequestBufferSize);
			Trace("maxReqSz:" << fProc->fRequestSizeLimit);
			Trace("fRequestBuffer.Length():" << fRequestBuffer.Length());
			return CheckReqBufferSize(Ios, fRequestBufferSize, line, clientInfo);
		}
	}
	return false;
}

bool RequestReader::ReadRequest(iostream &Ios, const Anything &clientInfo)
{
	StartTrace(RequestReader.ReadRequest);
	String line;

	Trace("======================= reading input ==================");

	// read the first line, this is special
	// We must return true even in the error cases because we want HTTPProcessor to display
	// the error message.
	bool ret = ReadLine(Ios, line, clientInfo);
	if ( !ret) {
		return false;
	}
	if ( ret ) {
		ret = HandleFirstLine(Ios, line, clientInfo);
		Trace("First line: [" << line << "]");
	}
	while ( ret ) {
		ret = ReadLine(Ios, line, clientInfo);
		// end of request
		if (line == ENDL ) {
			break;
		}

		// handle request lines
		if (!fHeader.DoParseHeaderLine(line)) {
			// correct error code? FIXME
			return DoHandleError(Ios, 400, "Bad Request", line, clientInfo);
		}
	}
	if ( fRequestBufferSize == 0 ) {
		return DoHandleError(Ios, 400, "Bad Request", line, clientInfo);
	}

#ifdef REQ_TRACING
	SysLog::WriteToStderr(String("<") << fRequestBuffer << ">\n");
#endif
	TraceAny(fHeader.GetInfo(), "RequestHeader:");
	return true;
}

bool RequestReader::CheckReqLineSize(iostream &Ios, long lineLength, const String &line, const Anything &clientInfo)
{
	StartTrace(RequestReader.CheckReqLineSize);
	if (lineLength > fProc->fLineSizeLimit) {
		Trace("Request Line Too Large : [" << lineLength << "] (max " << fProc->fLineSizeLimit << ")");
		return DoHandleError(Ios, 413, "Request Line Too Large", line, clientInfo);
	}
	return true;
}

bool RequestReader::CheckReqBufferSize(iostream &Ios, long lineLength, const String &line, const Anything &clientInfo)
{
	StartTrace(RequestReader.CheckReqBufferSize);
	if (lineLength > fProc->fRequestSizeLimit) {
		return DoHandleError(Ios, 413, "Request Too Large", line, clientInfo);
	}
	return true;
}

bool RequestReader::CheckReqURISize(iostream &Ios, long lineLength, const String &line, const Anything &clientInfo)
{
	StartTrace(RequestReader.CheckReqURISize);
	if (lineLength > fProc->fURISizeLimit) {
		String msg("Request-URI Too Long [");
		msg << lineLength << "].";

		return DoHandleError(Ios, 414, msg, line, clientInfo);
	}
	return true;
}

bool RequestReader::HandleFirstLine(iostream &Ios, String &line, const Anything &clientInfo)
{
	StartTrace(RequestReader.HandleFirstLine);
	long llen = line.Length();
	if (line == ENDL || !(line[long(llen-2)] == '\r' && line[long(llen-1)] == '\n')) {
		return DoHandleError(Ios, 400, "Bad Request", line, clientInfo);
	} else {
		line.Trim(llen - 2); // cut off ENDL
	}
	if (!CheckReqURISize(Ios, line.Length(), line, clientInfo)) {
		return false;
	}
	if (!ParseRequest(Ios, line, clientInfo)) {
		return false;
	}
	return true;
}

bool RequestReader::ParseRequest(iostream &Ios, String &line, const Anything &clientInfo)
{
	StartTrace(RequestReader.ParseRequest);
	Trace("Line:<" << line << ">");
	StringTokenizer st(line, ' ');
	String tok;

	fRequest["TraversialAttack"]	= 0;
	fRequest["WrongUrlPathEncoding"]	= 0;
	fRequest["WrongUrlArgsEncoding"]	= 0;
	if ( st.NextToken(tok) ) {
		// request type
		tok.ToUpper();
		if ( tok == "GET" || tok == "POST" ) { // FIXME: allow more methods..., make it configurable?
			fRequest["REQUEST_METHOD"] = tok;
		} else {
			return DoHandleError(Ios, 405, "Method Not Allowed", line, clientInfo);
		}
	} else {
		return DoHandleError(Ios, 405, "Method Not Allowed", line, clientInfo);
	}
	String cleanUrl;
	String urlPath;
	if ( st.NextToken(tok) ) {
		// extract request url path, without trailing ?
		long questPos = tok.StrChr('?');
		if (questPos != -1) {
			String urlPath(tok.SubString(0, questPos));
			Trace("UrlPath: questPos: " << questPos << "urlPath: " << urlPath);
			if (!VerifyUrlPath(Ios, urlPath, clientInfo)) {
				return false;
			}
			String urlArgs;
			// Do we have any args in original, unchanged request?
			bool hasArgs(questPos != tok.Length() - 1);
			if ( hasArgs ) {
				urlArgs = tok.SubString(questPos + 1, tok.Length());
				Trace("UrlArgs: questPos: " << questPos << "urlArgs: " << urlArgs);
				VerifyUrlArgs(urlArgs);
				cleanUrl << urlPath << "?" <<  urlArgs;
			} else {
				cleanUrl << urlPath;
			}
		} else {
			if (tok == "") {
				Trace("returning false");
				return false;
			}
			// No arguments supplied
			urlPath = tok;
			Trace("urlPath: " << urlPath);
			if (!VerifyUrlPath(Ios, urlPath, clientInfo)) {
				return false;
			}
			cleanUrl = urlPath;
		}
		fRequest["REQUEST_URI"] = cleanUrl;
		Trace("Resulting Url " << cleanUrl);
	} else {
		return DoHandleError(Ios, 405, "Method Not Allowed", line, clientInfo);
	}
	if ( st.NextToken(tok) ) {
		// request protocol
		fRequest["SERVER_PROTOCOL"] = tok;
	} else {
		return DoHandleError(Ios, 405, "Method Not Allowed", line, clientInfo);
	}
	TraceAny(fRequest, "Request");
	Trace("ParseRequest returning true");
	return true;
}

bool RequestReader::VerifyUrlPath(iostream &Ios, String &urlPath, const Anything &clientInfo)
{
	StartTrace(RequestReader.VerifyUrlPath);
	// Are all character which must be URL-encoded really encoded?
	if (URLUtils::CheckUrlEncoding(urlPath) == false) {
		fRequest["WrongUrlPathEncoding"] = 1;
	}
	Trace("DoURLDecoding: " <<  fProc->fDoURLDecoding);
	if ( fProc->fDoURLDecoding ) {
		String urlPathOrig(urlPath);
		URLUtils::URLCheckStatus eUrlCheckStatus;
		// Do we have a char > FF ? This is suspicious.
		urlPathOrig = urlPath;
		urlPath = URLUtils::ExhaustiveUrlDecode(urlPath, eUrlCheckStatus, false);
		if (eUrlCheckStatus == URLUtils::eSuspiciousChar) {
			// We are done, invalid request
			return DoHandleError(Ios, 400, "Bad Request", urlPathOrig, clientInfo);
		}
	}
	long notNormalizedLength(urlPath.Length());
	urlPath = URLUtils::CleanUpUriPath(urlPath);
	// "path" part of URL had to be normalized. This may indicate an attack.
	if ( urlPath.Length() != notNormalizedLength ) {
		fRequest["TraversialAttack"] = 1;
	}
	if ( fProc->fDoURLDecoding ) {
		urlPath = URLUtils::urlEncode(urlPath, fProc->fURLEncodeExclude);
	}
	return true;
}

bool RequestReader::VerifyUrlArgs(String &urlArgs)
{
	StartTrace(RequestReader.VerifyUrlArgs);
	// Are all character which must be URL-encoded really encoded?
	if (URLUtils::CheckUrlArgEncoding(urlArgs) == false) {
		fRequest["WrongUrlArgsEncoding"] = 1;
		return false;
	}
	return true;
}

Anything RequestReader::GetRequest()
{
	fRequest["header"] = fHeader.GetInfo();
	return fRequest;
}

// handle error
bool RequestReader::DoHandleError(iostream &Ios, long errcode, const String &msg, const String &line, const Anything &clientInfo)
{
	StartTrace(RequestReader.DoHandleError);
	Trace("Errcode: " << errcode << " Mesage: " << msg << " Faulty line: " << line);
	if ( !!Ios ) {
		Context ctx;

		Anything tmpStore = ctx.GetTmpStore();
		tmpStore["HTTPStatus"]["ResponseCode"] = errcode;
		fProc->DoError(Ios, msg, ctx);

		Ios << ENDL;
		Ios.flush();
		Ios.clear(ios::badbit);
	} else {
		Trace("Can not send error msg.");
	}
	DoLogError(errcode, msg, line, clientInfo);
	return false;
}

void RequestReader::DoLogError(long errcode, const String &msg, const String &line, const Anything &clientInfo)
{
	StartTrace(RequestReader.DoHandleError);
	// define SecurityLog according to the AppLog rules if you want to see this output.
	Context ctx;
	Anything tmpStore = ctx.GetTmpStore();
	tmpStore["REMOTE_ADDR"] = clientInfo["REMOTE_ADDR"];
	tmpStore["HTTPS"] = clientInfo["HTTPS"];
	tmpStore["Request"] = GetRequest();
	tmpStore["HttpStatusCode"] =  errcode;
	tmpStore["HttpResponseMsg"] = msg;
	tmpStore["FaultyRequestLine"] = line;
	AppLogModule::Log(ctx, "SecurityLog");
}
