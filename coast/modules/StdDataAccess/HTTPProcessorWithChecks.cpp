/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPProcessorWithChecks.h"
#include "HTTPProtocolReplyRenderer.h"
#include "AnyIterators.h"
#include "AppLog.h"
#include "AnythingUtils.h"
#include "StringStream.h"
#include "HTTPConstants.h"

RegisterRequestProcessor(HTTPProcessorWithChecks);

namespace {
	String const strGET("GET", Coast::Storage::Global());
	String const strPOST("POST", Coast::Storage::Global());

	void PutErrorMessageIntoContext(Context& ctx, long const errorcode, String const& msg, String const& content) {
		StartTrace(HTTPProcessor.PutErrorMessageIntoContext);
		Anything anyMessage;
		anyMessage[Coast::HTTP::constants::protocolCodeSlotname] = errorcode;
		anyMessage[Coast::HTTP::constants::protocolMsgSlotname] = HTTPProtocolReplyRenderer::DefaultReasonPhrase(errorcode); //!@FIXME: remove but create and use HTTPResponseMsgRenderer instead where needed, issue #245
		anyMessage["ErrorMessage"] = msg;
		anyMessage["FaultyContent"] = content;
		TraceAny(anyMessage, "generated error message");
		StorePutter::Operate(anyMessage, ctx, "Tmp", ctx.Lookup("RequestProcessorErrorSlot", "HTTPProcessor.Error"), true);
	}

	bool PostOrGetValuePresent(Context& ctx, ROAnything roaHeaders, String const& msg) {
		StartTrace(HTTPProcessor.CheckValues);
		String work(64L);
		AnyExtensions::Iterator<ROAnything> aValueIter(roaHeaders);
		ROAnything roaValue;
		while (aValueIter.Next(roaValue)) {
			work = roaValue.AsString().ToUpper();
			if ( work.StartsWith(strGET.cstr()) || work.StartsWith(strPOST.cstr()) ) {
				Trace("value [" << work << "] starts with either POST or GET which is suspicious");
				String line;
				aValueIter.SlotName(line);
				line.Append(": ").Append(roaValue.AsString());
				PutErrorMessageIntoContext(ctx, 400, msg, line);
				return true;
			}
		}
		return false;
	}
}

bool HTTPProcessorWithChecks::DoVerifyRequest(Context &ctx) {
	StartTrace(HTTPProcessorWithChecks.DoVerifyRequest);
	bool const rejectOnFailure = ctx.Lookup("RejectRequestsWithInvalidHeaders", 0L);
	ROAnything roaHeaders = ctx.Lookup("header");
	if ( ctx.Lookup("CheckHeaderFields", 1L) ) {
		if ( PostOrGetValuePresent(ctx, roaHeaders, "Possible SSL Renegotiation attack. A header contains a GET/POST request") && rejectOnFailure) {
			return false;
		}
		AnyExtensions::Iterator<ROAnything> bodyHeaderIter(ctx.Lookup("REQUEST_BODY"));
		while ( bodyHeaderIter.Next(roaHeaders) ) {
			if ( PostOrGetValuePresent(ctx, roaHeaders["header"], "Possible SSL Renegotiation attack. A multipart mime header (in POST) contains a GET/POST request") && rejectOnFailure) {
				return false;
			}
		}
	}
	return true;
	//	// extract request url path, without trailing ? or;
	//	long argDelimPos = tok.FirstCharOf("?;");
	//	if (argDelimPos != -1) {
	//		char argDelim = tok[argDelimPos];
	//		String urlPath(tok.SubString(0, argDelimPos));
	//		Trace("UrlPath: argDelimPos: " << argDelimPos << " argDelim: " << argDelim << "urlPath: " << urlPath);
	//		if (!VerifyUrlPath(ctx, Ios, urlPath)) {
	//			return false;
	//		}
	//		String urlArgs;
	//		// Do we have any args in original, unchanged request?
	//		bool hasArgs(argDelimPos != tok.Length() - 1);
	//		if ( hasArgs ) {
	//			urlArgs = tok.SubString(argDelimPos + 1, tok.Length());
	//			Trace("UrlArgs: argDelimPos: " << argDelimPos << " urlArgs: " << urlArgs);
	//			cleanUrl << urlPath << argDelim <<  urlArgs;
	//			if ( !VerifyUrlArgs(ctx, urlArgs) ) {
	//				String reason("Argument string (after ");
	//				reason << argDelim << ") was not correctly encoded. Request not rejected.";
	//				LogError(ctx, 0, reason, cleanUrl, "");
	//			}
	//		} else {
	//			cleanUrl << urlPath;
	//		}
	//	} else {
	//		if (tok == "") {
	//			return DoHandleError(ctx, Ios, 400, "No URL in request.", line);
	//		}
	//		// No arguments supplied
	//		url = tok;
	//		Trace("urlPath: " << url);
	//		if (!VerifyUrlPath(ctx, Ios, url)) {
	//			return false;
	//		}
	//		cleanUrl = url;
	//	}
}
void HTTPProcessorWithChecks::DoHandleVerifyError(std::ostream &reply, Context &ctx) {
	StartTrace(HTTPProcessorWithChecks.DoHandleVerifyError);
	OStringStream ostr;
	ctx.DebugStores("Stores after verify error", ostr, true);
	Trace(ostr.str());
	// handle errors of HTTPRequestReader and HTTPProcessor
//	bool HTTPRequestReader::DoHandleError(Context &ctx, std::iostream &Ios, long errcode, const String &reason, const String &line, bool reject, const String &msg)
//	{
//		StartTrace(HTTPRequestReader.DoHandleError);
//		Trace("Errcode: [" << errcode << "] Message: [" << msg << "] Faulty line: [" << line << "] reject: [" << reject << "]");
//		if ( reject ) {
//			if ( !!Ios ) {
//				Anything anyErrCode = errcode;
//				StorePutter::Operate(anyErrCode, ctx, "Tmp", String("HTTPStatus.").Append(Coast::HTTP::constants::protocolCodeSlotname));
//				Error(Ios, msg, ctx);
//				Ios << ENDL;
//				Ios.flush();
//				Ios.clear(std::ios::badbit);
//			} else {
//				Trace("Can not send error msg.");
//			}
//		}
//		LogError(ctx, errcode, reason, line, msg);
//		// If we do only logging, the request must be valid for further processing to be successful.
//		bool ret = !reject;
//		Trace("Returning: " << ret);
//		return ret;
//	}
}

//!Verify RFC 1738 compliance -> now rfc 2396
//bool HTTPRequestReader::VerifyUrlPath(Context &ctx, std::iostream &Ios, String &urlPath)
//{
//	StartTrace(HTTPRequestReader.VerifyUrlPath);
//
//	Coast::URLUtils::URLCheckStatus eUrlCheckStatus = Coast::URLUtils::eOk;
//	String urlPathOrig = urlPath;
//	// Are all chars which must be URL-encoded really encoded?
//	if (Coast::URLUtils::CheckUrlEncoding(urlPath, ctx.Lookup("CheckUrlEncodingOverride", "")) == false) {
//		return DoHandleError(ctx, Ios, 400, "Not all unsafe chars URL encoded", urlPathOrig);
//	}
//	if (ctx.Lookup("URLExhaustiveDecode", 0L)) {
//		urlPath = Coast::URLUtils::ExhaustiveUrlDecode(urlPath, eUrlCheckStatus, false);
//	} else {
//		urlPath = Coast::URLUtils::urlDecode(urlPath, eUrlCheckStatus, false);
//	}
//	if (eUrlCheckStatus == Coast::URLUtils::eSuspiciousChar) {
//		// We are done, invalid request
//		return DoHandleError(ctx, Ios, 400, "Encoded char above 0x255 detected", urlPathOrig);
//	}
//	if ( Coast::URLUtils::CheckUrlPathContainsUnsafeChars(urlPath, ctx.Lookup("CheckUrlPathContainsUnsafeCharsOverride", ""),
//			ctx.Lookup("CheckUrlPathContainsUnsafeCharsAsciiOverride", ""),
//			!(ctx.Lookup("CheckUrlPathContainsUnsafeCharsDoNotCheckExtendedAscii", 0L))) ) {
//		return DoHandleError(ctx, Ios, 400, "Decoded URL path contains unsafe char", urlPathOrig);
//	}
//	// "path" part of URL had to be normalized. This may indicate an attack.
//	String normalizedUrl =  Coast::URLUtils::CleanUpUriPath(urlPath);
//	if ( urlPath.Length() !=  normalizedUrl.Length() ) {
//		if ( ctx.Lookup("FixDirectoryTraversial", 0L) ) {
//			// alter the original url
//			urlPathOrig = Coast::URLUtils::urlEncode(normalizedUrl, ctx.Lookup("URLEncodeExclude", "/?"));
//			LogError(ctx, 0, "Directory traversial attack detected and normalized. Request not rejected because of config settings",
//					   urlPathOrig, "");
//		} else {
//			return DoHandleError(ctx, Ios, 400, "Directory traversial attack", urlPathOrig);
//		}
//	}
//	urlPath = urlPathOrig;
//
//	return true;
//}
//
//!Verify t RFC 1738 compliance
//bool HTTPRequestReader::VerifyUrlArgs(Context &ctx, String &urlArgs)
//{
//	StartTrace(HTTPRequestReader.VerifyUrlArgs);
//	// Are all character which must be URL-encoded really encoded?
//	return Coast::URLUtils::CheckUrlArgEncoding(urlArgs, ctx.Lookup("CheckUrlArgEncodingOverride", ""));
//}
