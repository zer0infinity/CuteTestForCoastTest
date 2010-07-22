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

RegExpAction::RegExpAction(const char *name) : Action(name) { }

RegExpAction::~RegExpAction() { }

bool RegExpAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(RegExpAction.DoExecAction);

	ROAnything roaText;
	ROAnything roaPattern;
	ROAnything roaDest;
	if ( !(config.LookupPath(roaText, "Text") && config.LookupPath(roaPattern, "Pattern") && config.LookupPath(roaDest, "Destination")) ) {
		return false;
	}

	// check if the string is already a string value or if it equals a "/Lookup ...", in which case has to be rendered to a string
	String sText;
	if ( config["DoRender"].AsBool(true) ) {
		String renderedValue;
		Renderer::RenderOnString(renderedValue, ctx, roaText);
		Trace("Rendered value: [" << renderedValue << "]");
		sText = renderedValue;
	} else {
		sText = roaText.AsString();
	}

	String sPattern = roaPattern.AsString();
	RE aRE(sPattern);
	if ( aRE.IsValid() ) {
		String sMatchedString;
		// does the pattern match from position 0 within search string (but doesn't return the matched String)
		if ( aRE.ContainedIn(sText) ) {
			// get the whole matched string
			sMatchedString = aRE.GetMatch();

			// store the matched string in the specified Destination.Store and Destination.Slot
			Anything destConfig;
			destConfig = roaDest.DeepClone();

			Anything anyToStore(sMatchedString);
			StorePutter::Operate(anyToStore, ctx, destConfig);

			Trace("String: '" << sText << ", using pattern " << sPattern << ", returned string: '" << sMatchedString << "'" );
		} else {
			Trace("Couldn't match pattern " << sPattern << " in String " << sText );
			return false;
		}
	} else {
		Trace("Pattern " << sPattern << " is invalid.");
	}

	return true;
}
