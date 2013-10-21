/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "RegExpReplaceRenderer.h"
#include "RE.h"
#include "AnyIterators.h"

RegisterRenderer(RegExpReplaceRenderer);

namespace {
	namespace constants {
		const char * const textSlotName = "Text";
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

void RegExpReplaceRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(RegExpReplaceRenderer.RenderAll);
	ROAnything roaText, roaExpressions;
	if (!config.LookupPath(roaText, constants::textSlotName, '\000') || !config.LookupPath(roaExpressions, constants::expressionsSlotName, '\000')) {
		return;
	}
	// check if the string is already a string value or if it equals a "/Lookup ...", in which case has to be rendered to a string
	String sText = getSimpleOrRenderedString(ctx, roaText);
	searchReplaceExpressions(ctx, sText, roaExpressions);
	Trace("returning [" << sText << "]");
	reply << sText;
}
