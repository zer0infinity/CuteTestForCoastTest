/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RegExpAction.h"

//--- standard modules used ----------------------------------------------------
#include "Context.h"
#include "Renderer.h"
#include "AnythingUtils.h"
#include "Dbg.h"
#include "RE.h"

//---- RegExpAction ---------------------------------------------------------------
RegisterAction(RegExpAction);
bool RegExpAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(RegExpAction.DoExecAction);
	ROAnything roaText, roaPattern, roaDest, roaMatchFlags;
	if ( !config.LookupPath(roaText, "Text", '\000') || !config.LookupPath(roaPattern, "Pattern", '\000') || !config.LookupPath(roaDest, "Destination", '\000') ) {
		return false;
	}
	// check if the string is already a string value or if it equals a "/Lookup ...", in which case has to be rendered to a string
	String sText(1024L);
	Renderer::RenderOnString(sText, ctx, roaText);
	String sPattern( roaPattern.AsString() );
	Trace("String [" << sText << "] using pattern [" << sPattern << "]");
	config.LookupPath(roaMatchFlags, "MatchFlags", '\000');
	RE aRE(sPattern, static_cast<RE::eMatchFlags>(roaMatchFlags.AsLong(0L)));
	if ( aRE.IsValid() ) {
		// does the pattern match from position 0 within search string (but doesn't return the matched String)
		if ( aRE.ContainedIn(sText) ) {
			Anything anyToStore;
			for ( long lMatch=0L, lTotalMatches = aRE.GetRegisterCount(); lMatch < lTotalMatches; ++lMatch ) {
				if ( lTotalMatches == 1L ) {
					anyToStore = aRE.GetMatch(lMatch);
				} else {
					anyToStore.Append(aRE.GetMatch(lMatch));
				}
			}
			TraceAny(anyToStore, "matched groups");
			StorePutter::Operate(anyToStore, ctx, roaDest);
			return true;
		} else {
			Trace("No matches found");
		}
	} else {
		Trace("Pattern " << sPattern << " is invalid.");
	}
	return false;
}
