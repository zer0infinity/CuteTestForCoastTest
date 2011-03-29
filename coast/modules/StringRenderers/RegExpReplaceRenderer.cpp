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
	String getSimpleOrRenderedString(Context &ctx, ROAnything roaConfig) {
		if (roaConfig.GetType() == AnyArrayType) {
			return Renderer::RenderToString(ctx, roaConfig);
		}
		return roaConfig.AsString();
	}
}

void RegExpReplaceRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(RegExpReplaceRenderer.RenderAll);
	ROAnything roaText, roaExpressions;
	if (!config.LookupPath(roaText, "Text", '\000') || !config.LookupPath(roaExpressions, "Expressions", '\000')) {
		return;
	}
	TraceAny(roaExpressions, "list of expressions");
	// check if the string is already a string value or if it equals a "/Lookup ...", in which case has to be rendered to a string
	String sText = getSimpleOrRenderedString(ctx, roaText);
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
		if (roaEntry.LookupPath(roaPattern, "Pattern", '\000') || (isSimplePattern = roaEntry.LookupPath(roaPattern, "SimplePattern",
				'\000'))) {
			sPattern = getSimpleOrRenderedString(ctx, roaPattern);
			if (isSimplePattern) {
				sPattern = RE::SimplePatternToFullRegularExpression(sPattern);
			}
		}
		String sReplacement = getSimpleOrRenderedString(ctx, roaEntry["Replacement"]);
		RE aRE(sPattern, static_cast<RE::eMatchFlags> (roaEntry["MatchFlags"].AsLong(0L)));
		Trace("String [" << sText << "], applying pattern [" << sPattern << "]");
		sText = aRE.Subst(sText, sReplacement, roaEntry["ReplaceAll"].AsBool(true));
	} while (expressionIterator.Next(roaEntry));
	Trace("returning [" << sText << "]");
	reply << sText;
}
