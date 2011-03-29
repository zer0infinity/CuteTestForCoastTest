/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RegExpSearchReplaceResultMapper.h"
#include "RE.h"
#include "AnyIterators.h"
#include "Renderer.h"
#include "StringStream.h"

namespace {
	const char * _Expressions = "Expressions";
	const char * _Pattern = "Pattern";
	const char * _SimplePattern = "SimplePattern";
	const char * _Replacement = "Replacement";
	const char * _MatchFlags = "MatchFlags";
	const char * _ReplaceAll = "ReplaceAll";

	String getSimpleOrRenderedString(Context &ctx, ROAnything roaConfig) {
		if (roaConfig.GetType() == AnyArrayType) {
			return Renderer::RenderToString(ctx, roaConfig);
		}
		return roaConfig.AsString();
	}
	void searchReplaceExpressions(Context &ctx, String &sText, ROAnything roaExpressions) {
		StartTrace(RegExpSearchReplaceResultMapper.searchReplaceExpressions);
		TraceAny(roaExpressions, "list of expressions");
		// check if we have multiple replacements by testing if first entry in Expressions is an AnyArrayType or not
		bool singleExpression = not (roaExpressions[0L].GetType() == AnyArrayType);
		AnyExtensions::Iterator<ROAnything> expressionIterator(singleExpression ? ROAnything() : roaExpressions);
		ROAnything roaEntry = roaExpressions;
		if (not singleExpression) {
			expressionIterator.Next(roaEntry);
		}
		TraceAny(roaEntry, (singleExpression?"single ":"prefetched ") << "expression configuration" );
		do {
			String sPattern;
			ROAnything roaPattern, roaMatchFlags;
			bool isSimplePattern = false;
			if (roaEntry.LookupPath(roaPattern, _Pattern, '\000') || (isSimplePattern = roaEntry.LookupPath(roaPattern, _SimplePattern,
					'\000'))) {
				sPattern = getSimpleOrRenderedString(ctx, roaPattern);
				if (isSimplePattern) {
					sPattern = RE::SimplePatternToFullRegularExpression(sPattern);
				}
			}
			String sReplacement = getSimpleOrRenderedString(ctx, roaEntry[_Replacement]);
			RE aRE(sPattern, static_cast<RE::eMatchFlags> (roaEntry[_MatchFlags].AsLong(0L)));
			Trace("String [" << sText << "], applying pattern [" << sPattern << "]");
			sText = aRE.Subst(sText, sReplacement, roaEntry[_ReplaceAll].AsBool(true));
		} while (expressionIterator.Next(roaEntry));
		Trace("replaced text [" << sText << "]");
	}
}
//---- RegExpSearchReplaceResultMapper ----------------------------------------------------------------
RegisterResultMapper(RegExpSearchReplaceResultMapper);

bool RegExpSearchReplaceResultMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RegExpSearchReplaceResultMapper.DoPutAny, "key [" << NotNull(key) << "]");
	if (value.GetType() != AnyArrayType) {
		String sText = value.AsString();
		ROAnything roaExpressions;
		if (Lookup(_Expressions, roaExpressions, '\000')) {
			searchReplaceExpressions(ctx, sText, roaExpressions);
			value = sText;
		}
	}
	return ResultMapper::DoPutAny(key, value, ctx, script);
}

bool RegExpSearchReplaceResultMapper::DoPutStream(const char *key, std::istream & is, Context & ctx, ROAnything script) {
	StartTrace1(RegExpSearchReplaceResultMapper.DoPutStream, "key [" << NotNull(key) << "]");
	OStringStream content;
	long lRecv = 0, lToRecv = 2048;
	l_long lTotalReceived = 0LL;
	while (NSStringStream::PlainCopyStream2Stream(&is, content, lRecv, lToRecv) && lRecv == lToRecv)
		lTotalReceived += lRecv;
	lTotalReceived += lRecv;
	Trace("total characters read: " << lTotalReceived);
	Anything value = content.str();
	return DoPutAny(key, value, ctx, script);
}
