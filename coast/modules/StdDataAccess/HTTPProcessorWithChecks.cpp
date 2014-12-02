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
#include "Timers.h"
#include "URLUtils.h"

RegisterRequestProcessor(HTTPProcessorWithChecks);

namespace {
	void LogWarning(Context& ctx, long const errorcode, String const& msg, String const& content, String const& component) {
		StartTrace(HTTPProcessorWithChecks.LogWarning);
		Anything anyMessage = coast::http::GenerateErrorMessageAny(ctx, errorcode, msg, content, component);
		Context::PushPopEntry<Anything> aLogEntry(ctx, "HTTPProcessorWarning", anyMessage, ctx.Lookup("RequestProcessorErrorSlot", "HTTPProcessor.Error"));
		AppLogModule::Log(ctx, "SecurityLog", AppLogModule::eWARNING);
	}
	bool PostOrGetValuePresent(Context& ctx, ROAnything roaHeaders, String const& msg) {
		StartTrace(HTTPProcessorWithChecks.PostOrGetValuePresent);
		String work(64L);
		AnyExtensions::Iterator<ROAnything> aValueIter(roaHeaders);
		ROAnything roaValue;
		while (aValueIter.Next(roaValue)) {
			work = roaValue.AsString().ToUpper();
			if ( work.StartsWith(coast::http::constants::getMethodSlotname) || work.StartsWith(coast::http::constants::postMethodSlotname) ) {
				Trace("value [" << work << "] starts with either POST or GET which is suspicious");
				String line;
				aValueIter.SlotName(line);
				line.Append(": ").Append(roaValue.AsString());
				coast::http::PutErrorMessageIntoContext(ctx, 400, msg, line, "HTTPProcessorWithChecks::DoVerifyRequest");
				return true;
			}
		}
		return false;
	}
	bool HasUriPathResolved(Context &ctx, String &urlPath) {
		StartTrace(HTTPProcessorWithChecks.HasUriPathResolved);
		// "path" part of URL had to be normalized. This may indicate an attack.
		String normalizedUrl = coast::urlutils::CleanUpUriPath(urlPath);
		if (urlPath.Length() != normalizedUrl.Length()) {
			if (ctx.Lookup("FixDirectoryTraversial", 0L)) {
				// alter the original url
				coast::http::PutErrorMessageIntoContext(ctx, 200, "Directory traversal attack detected and normalized. "
						"Request not rejected because of FixDirectoryTraversial setting", urlPath, "HTTPProcessorWithChecks::DoPrepareContextRequest");
				urlPath = coast::urlutils::urlEncode(normalizedUrl, ctx.Lookup("URLEncodeExclude", "/?"));
			} else {
				coast::http::PutErrorMessageIntoContext(ctx, 400, "Directory traversal attack", urlPath, "HTTPProcessorWithChecks::DoPrepareContextRequest");
				return false;
			}
		}
		return true;
	}
	bool HasAllPathCharactersUrlEncoded(Context &ctx, String const &urlPath) {
		StartTrace(HTTPProcessorWithChecks.HasAllPathCharactersUrlEncoded);
		// Are all chars which must be URL-encoded really encoded? (RFC 1738 -> now rfc 2396)
		return coast::urlutils::CheckUrlEncoding(urlPath, ctx.Lookup("CheckUrlEncodingOverride", ""));
	}
	bool HasAllArgsCharactersUrlEncoded(Context &ctx, String const &urlArgs) {
		StartTrace(HTTPProcessorWithChecks.HasAllArgsCharactersUrlEncoded);
		// Are all chars which must be URL-encoded really encoded? (RFC 1738 -> now rfc 2396)
		return coast::urlutils::CheckUrlArgEncoding(urlArgs, ctx.Lookup("CheckUrlArgEncodingOverride", ""));
	}
	bool UrlPathContainsUnsafeChars(Context &ctx, String const &urlPath) {
		StartTrace(HTTPProcessorWithChecks.UrlPathContainsUnsafeChars);
		// according (RFC 1738 -> now rfc 2396)
		return coast::urlutils::CheckUrlPathContainsUnsafeChars(urlPath, ctx.Lookup("CheckUrlPathContainsUnsafeCharsOverride", ""),
				ctx.Lookup("CheckUrlPathContainsUnsafeCharsAsciiOverride", ""),
				!(ctx.Lookup("CheckUrlPathContainsUnsafeCharsDoNotCheckExtendedAscii", 0L)));
	}
	bool DecodeAllUrlCharacters(Context &ctx, String &urlPath) {
		StartTrace(HTTPProcessorWithChecks.DecodeAllUrlCharacters);
		// Are all chars which must be URL-encoded really encoded?
		coast::urlutils::URLCheckStatus eUrlCheckStatus = coast::urlutils::eOk;
		String normalizedUrl;
		if (ctx.Lookup("URLExhaustiveDecode", 0L)) {
			normalizedUrl = coast::urlutils::ExhaustiveUrlDecode(urlPath, eUrlCheckStatus, false);
		} else {
			normalizedUrl = coast::urlutils::urlDecode(urlPath, eUrlCheckStatus, false);
		}
		if (eUrlCheckStatus == coast::urlutils::eSuspiciousChar) {
			// We are done, invalid request
			coast::http::PutErrorMessageIntoContext(ctx, 400, "Encoded char above 0x255 detected", urlPath, "HTTPProcessorWithChecks::DoPrepareContextRequest");
			return false;
		}
		urlPath = normalizedUrl;
		return true;
	}
	struct RequestURISplitHelper {
		Anything &fRequest;
		String fPath, fArgSep, fArgs;
		RequestURISplitHelper(Anything &request) : fRequest(request) {
			split();
		}
		void split() {
			StartTrace(RequestURISplitHelper.split);
			fPath = fRequest["REQUEST_URI"].AsString();
			Trace("unmodified URI [" << fPath << "]");
			// extract request url path, without trailing ? or;
			long argDelimPos = fPath.FirstCharOf("?;");
			if (argDelimPos != -1) {
				if (argDelimPos != fPath.Length()-1) {
					fArgSep = fPath.SubString(argDelimPos, 1);
					fArgs = fPath.SubString(argDelimPos + 1, fPath.Length());
				}
				fPath = fPath.SubString(0, argDelimPos);
			}
			Trace("UrlPath: argDelimPos: " << argDelimPos << " argDelim: " << fArgSep << " Path: " << fPath << " Args: " << fArgs);
		}
		String merge() {
			StartTrace(RequestURISplitHelper.merge);
			fPath.Append(fArgSep).Append(fArgs);
			Trace("  modified URI [" << fPath << "]");
			return fPath;
		}
	};
}

bool HTTPProcessorWithChecks::DoPrepareContextRequest(std::iostream &Ios, Context &ctx, Anything &request, HTTPRequestReader &reader) {
	StartTrace(HTTPProcessorWithChecks.DoPrepareContextRequest);
	MethodTimer(HTTPProcessorWithChecks.DoPrepareContextRequest, "Preparing request for context", ctx);
	// modify request line arguments as requested

	RequestURISplitHelper splitHelper(request);
	if ( not HasAllPathCharactersUrlEncoded(ctx, splitHelper.fPath) ) {
		coast::http::PutErrorMessageIntoContext(ctx, 400, "Not all unsafe chars URL encoded", splitHelper.fPath, "HTTPProcessorWithChecks::DoVerifyRequest");
		return false;
	}
	if ( not HasAllArgsCharactersUrlEncoded(ctx, splitHelper.fArgs) ) {
		String reason("Argument string (after ");
		reason << splitHelper.fArgSep << ") was not correctly encoded. Request not rejected.";
		coast::http::PutErrorMessageIntoContext(ctx, 200, reason, splitHelper.fArgs, "HTTPProcessorWithChecks::DoVerifyRequestv");
	}
	if ( not DecodeAllUrlCharacters(ctx, splitHelper.fPath) ) {
		return false;
	}
	if ( UrlPathContainsUnsafeChars(ctx, splitHelper.fPath) ) {
		coast::http::PutErrorMessageIntoContext(ctx, 400, "Decoded URL path contains unsafe char", splitHelper.fPath, "HTTPProcessorWithChecks::DoVerifyRequest");
		return false;
	}
	if ( not HasUriPathResolved(ctx, splitHelper.fPath) ) {
		return false;
	}
	request["REQUEST_URI"] = splitHelper.merge();

	return HTTPProcessor::DoPrepareContextRequest(Ios, ctx, request, reader);
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
}

bool HTTPProcessorWithChecks::DoHandleReadInputError(std::iostream &Ios, Context &ctx) {
	StartTrace(HTTPProcessorWithChecks.DoHandleReadInputError);
	return HTTPProcessor::DoHandleReadInputError(Ios, ctx);
}

bool HTTPProcessorWithChecks::DoHandleVerifyRequestError(std::iostream &Ios, Context &ctx) {
	StartTrace(HTTPProcessorWithChecks.DoHandleVerifyRequestError);
	return HTTPProcessor::DoHandleVerifyRequestError(Ios, ctx);
}
