/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "HTTPMapper.h"
#include "Timers.h"
#include "CacheHandler.h"
#include "AnyIterators.h"
#include "HTTPConstants.h"

RegisterParameterMapper(HTTPHeaderParameterMapper);

namespace {
	namespace constants {
		const char * const groupName = "HeaderMapperCache";
		const char * const suppressName = "Suppress";
	}

	void SuppressListToUpper(ROAnything suppressList, Anything &suppressListToUpper) {
		const long size = suppressList.GetSize();
		for (long i = 0; i < size; ++i) {
			if (suppressList[i].GetType() == AnyArrayType) {
				SuppressListToUpper(suppressList[i], suppressListToUpper);
			} else {
				suppressListToUpper[suppressList[i].AsString().ToUpper()] = 1L;
			}
		}
	}
}

bool HTTPHeaderParameterMapper::DoInitialize() {
	StartTrace1(HTTPHeaderParameterMapper.DoInitialize, "cat <" << fCategory << "> name <" << fName << ">");
	ROAnything roaSuppressList(fConfig[constants::suppressName]);
	Anything suppresslist;
	SuppressListToUpper(roaSuppressList, suppresslist);
	TraceAny(suppresslist, "suppress list to cache");
	if (!suppresslist.IsNull()) {
		Anything toCache = Anything(Anything::ArrayMarker());
		toCache[constants::suppressName] = suppresslist;
		AnythingLoaderPolicy loader(toCache);
		ROAnything roaList = CacheHandler::instance().Load(constants::groupName, GetName(), &loader);
		TraceAny(roaList, "cached suppress list");
	}
	return true;
}

bool HTTPHeaderParameterMapper::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
	StartTrace1(HTTPHeaderParameterMapper.DoLookup, "fName <" << GetName() << ">");
	// check first in cache with this object name as prefix
	bool bValueFound = false;
	if (String(key).StartsWith(constants::suppressName)) {
		ROAnything roaCache = CacheHandler::instance().Get(constants::groupName, GetName());
		TraceAny(roaCache, "result from cache for [" << GetName() << "]");
		if (!roaCache.IsNull()) {
			bValueFound = roaCache.LookupPath(result, key);
			TraceAny(result, "result at [" << key << "]");
		}
	}
	// delegate further if not found yet
	if (!bValueFound) {
		bValueFound = HierarchConfNamed::DoLookup(key, result, delim, indexdelim);
		TraceAny(result, "uncached result of parent DoLookup delegation at key [" << key << "]");
	}
	return bValueFound;
}

bool HTTPHeaderParameterMapper::DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything info) {
	StartTrace1(HTTPHeaderParameterMapper.DoGetStream, "Key:<" << NotNull(key) << ">");
	bool mapSuccess = true;
	ROAnything headerfields(ctx.Lookup(key));
	TraceAny(headerfields, "Headerfields available for key " << key);
	if (!headerfields.IsNull()) {
		String strFieldName;
		ROAnything roaValue;
		AnyExtensions::Iterator<ROAnything> headerIterator(headerfields);
		while (headerIterator.Next(roaValue)) {
			if (headerIterator.SlotName(strFieldName)) {
				strFieldName.ToUpper();
				String strKey(constants::suppressName);
				strKey.Append('.').Append(strFieldName);
				Trace("trying to Lookup [" << strKey << "] in own or config");
				// map non suppressed headerfields
				if (Lookup(strKey).AsLong(0L) == 0L) {
					Anything value;
					ROAnything rvalue;
					Trace("slot: " << strFieldName);
					if (!Get(strFieldName, value, ctx)) {
						rvalue = roaValue;
					} else {
						rvalue = value;
					}
					coast::http::putHeaderFieldToStream(os, ctx, strFieldName, rvalue);
				}
			}
		}
	} else {
		TraceAny(ctx.GetTmpStore(), "no headers, get ReqHeader in tmp store:");
		String strHeaderfields;
		if ((mapSuccess = Get("ReqHeader", strHeaderfields, ctx))) {
			os << strHeaderfields;
		}
	}
	Trace("retval: " << mapSuccess);
	return mapSuccess;
}
RegisterResultMapper(HTTPBodyResultMapper);
RegisterResultMapperAlias(HTTPBodyMapper, HTTPBodyResultMapper);

bool HTTPBodyResultMapper::DoFinalPutStream(const char *key, std::istream &is, Context &ctx) {
	StartTrace(HTTPBodyResultMapper.DoFinalPutStream);
	DAAccessTimer(HTTPBodyResultMapper.DoFinalPutStream, "", ctx);
	String body;
	ReadBody(body, is, ctx);
	Anything anyVal(body);
	return DoFinalPutAny(key, anyVal, ctx);
}

void HTTPBodyResultMapper::ReadBody(String &body, std::istream &is, Context &ctx) {
	StartTrace(HTTPBodyResultMapper.ReadBody);

	long contentLength = ctx.Lookup("Mapper.content-length", -1L);
	Trace("contentLength: " << contentLength);
	if (contentLength > -1) {
		body.Append(is, contentLength);
	} else {
		char c;
		while (is.get(c).good()) {
			body.Append(c);
		}
	}
	Trace("Body[" << body.Length() << "]");
	Trace("<" << body << ">");
}
RegisterParameterMapper(HTTPBodyParameterMapper);
RegisterParameterMapperAlias(HTTPBodyMapper, HTTPBodyParameterMapper);
bool HTTPBodyParameterMapper::DoFinalGetStream(const char *key, std::ostream &os, Context &ctx) {
	StartTrace1(HTTPBodyParameterMapper.DoFinalGetStream, NotNull(key));
	bool mapSuccess = true;
	ROAnything params(ctx.Lookup(key)); //!@FIXME ??: use Get(key,any,ctx) instead?
	if (!params.IsNull()) {
		long bPSz = params.GetSize();
		String value;
		for (long i = 0; i < bPSz; ++i) {
			const char *lookupVal = params.SlotName(i);
			if (!lookupVal) {
				lookupVal = params[i].AsCharPtr("");
			}
			if (lookupVal && (mapSuccess = Get(lookupVal, value, ctx))) {
				Trace("Param[" << lookupVal << "]=<" << value << ">");
				os << lookupVal << "=" << value;
				if (i < (bPSz - 1)) {
					os << "&";
				}
			} else {
				mapSuccess = true;
				os << lookupVal;
			}
			value.clear();
		}
	} else {
		String bodyParams;
		if ((mapSuccess = Get(key, bodyParams, ctx))) {
			os << bodyParams;
		}
	}
	Trace("retval: " << mapSuccess);
	return mapSuccess;
}
