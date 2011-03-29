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
//---- RegExpReplaceRenderer ---------------------------------------------------------------
RegisterRenderer(RegExpReplaceRenderer);

namespace {
	const char * _Text = "Text";
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

void RegExpReplaceRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(RegExpReplaceRenderer.RenderAll);
	ROAnything roaText, roaExpressions;
	if (!config.LookupPath(roaText, _Text, '\000') || !config.LookupPath(roaExpressions, _Expressions, '\000')) {
		return;
	}
	// check if the string is already a string value or if it equals a "/Lookup ...", in which case has to be rendered to a string
	String sText = getSimpleOrRenderedString(ctx, roaText);
	searchReplaceExpressions(ctx, sText, roaExpressions);
	Trace("returning [" << sText << "]");
	reply << sText;
}
