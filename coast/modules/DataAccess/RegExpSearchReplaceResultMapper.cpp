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
	namespace constants {
		const char * const expressionsSlotName = "Expressions";
		const char * const patternSlotName = "Pattern";
		const char * const simplePatternSlotName = "SimplePattern";
		const char * const replacementSlotName = "Replacement";
		const char * const matchFlagsSlotName = "MatchFlags";
		const char * const replaceAllSlotName = "ReplaceAll";
	}

	String getSimpleOrRenderedString(Context &ctx, ROAnything roaConfig) {
		if (roaConfig.GetType() == AnyArrayType) {
			return Renderer::RenderToString(ctx, roaConfig);
		}
		return roaConfig.AsString();
	}
	void searchReplaceExpressions(Context &ctx, String &sText, ROAnything roaExpressions) {
		StartTrace(RegExpSearchReplaceResultMapper.searchReplaceExpressions);
		SubTraceAny(TraceExpressionList, roaExpressions, "list of expressions");
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
			if (roaEntry.LookupPath(roaPattern, constants::patternSlotName, '\000') || (isSimplePattern = roaEntry.LookupPath(roaPattern, constants::simplePatternSlotName,
					'\000'))) {
				sPattern = getSimpleOrRenderedString(ctx, roaPattern);
				if (isSimplePattern) {
					sPattern = RE::SimplePatternToFullRegularExpression(sPattern);
				}
			}
			String sReplacement = getSimpleOrRenderedString(ctx, roaEntry[constants::replacementSlotName]);
			RE aRE(sPattern, static_cast<RE::eMatchFlags> (roaEntry[constants::matchFlagsSlotName].AsLong(0L)));
			SubTrace(traceText, "original text [" << sText << "]");
			Trace("applying pattern [" << sPattern << "]");
			sText = aRE.Subst(sText, sReplacement, roaEntry[constants::replaceAllSlotName].AsBool(true));
		} while (expressionIterator.Next(roaEntry));
		SubTrace(traceText, "replaced text [" << sText << "]");
	}
}

RegisterResultMapper(RegExpSearchReplaceResultMapper);

bool RegExpSearchReplaceResultMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RegExpSearchReplaceResultMapper.DoPutAny, "key [" << NotNull(key) << "]");
	if (value.GetType() != AnyArrayType) {
		String sText = value.AsString();
		ROAnything roaExpressions;
		if (Lookup(constants::expressionsSlotName, roaExpressions, '\000')) {
			searchReplaceExpressions(ctx, sText, roaExpressions);
			value = sText;
		}
	}
	SubTraceAny(traceScript, script, "script to further process with key [" << key << "]");
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
