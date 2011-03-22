/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SplitCookieResultMapper.h"
#include "AnyIterators.h"
#include "AnythingUtils.h"

namespace {
	const char cookieValuesSeparator = ';';
	const char lookupPathSep = ',';
	const char keyValSeparator = '=';
	const char *valueSlotName = "_value_";
	const char *attrSlotName = "_attrs_";

	String splitCookie(String const& strCookie, Anything &anyNamedCookie) {
		StartTrace(SplitCookieResultMapper.splitCookie);
		String strKeyValue(64L), strKey(32L), strValue(64L);
		String cookieName(32L);
		StringTokenizer semiTokenizer(strCookie.cstr(), cookieValuesSeparator);
		while ( semiTokenizer.NextToken(strKeyValue) ) {
			StringTokenizer valueTokenizer(strKeyValue, keyValSeparator);
			if ( valueTokenizer.NextToken(strKey) ) {
				strKey.TrimWhitespace();
				strValue = valueTokenizer.GetRemainder();
				if ( !cookieName.Length() ) {
					cookieName = strKey;
					if ( strValue.Length() ) {
						anyNamedCookie[valueSlotName] = strValue;
					}
				} else {
					if ( strValue.Length() ) {
						anyNamedCookie[attrSlotName][strKey] = strValue;
					} else {
						anyNamedCookie[attrSlotName].Append(strKey);
					}
				}
			}
		}
		return cookieName;
	}
}

RegisterResultMapper(SplitCookieResultMapper);

bool SplitCookieResultMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(SplitCookieResultMapper.DoPutAny, NotNull(key));
	ROAnything roaCookie;
	String strCookie(128L), strCookieName;
	AnyExtensions::Iterator<ROAnything> cookieIterator(value);
	bool result = true;
	while ( cookieIterator.Next(roaCookie) ) {
		strCookie = roaCookie.AsString();
		Trace("current cookie entry [" << strCookie << "]");
		Anything anyCookie;
		strCookieName = splitCookie(strCookie, anyCookie);
		TraceAny(anyCookie, "putting away named cookie [" << strCookieName << "]");
		result = ResultMapper::DoPutAny(strCookieName, anyCookie, ctx, script);
	}
	return result;
}
