/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringsEqualAction.h"
#include "Renderer.h"
#include "Dbg.h"

//---- StringsEqualAction ---------------------------------------------------------------
RegisterAction(StringsEqualAction);

StringsEqualAction::StringsEqualAction(const char *name) : Action(name) { }

StringsEqualAction::~StringsEqualAction() { }

bool StringsEqualAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(StringsEqualAction.DoExecAction);

	TraceAny(config, "config");

	String string1, string2;
	ROAnything roaSlotConfig;

	if (config.LookupPath(roaSlotConfig, "String1")) {
		Renderer::RenderOnString(string1, ctx, roaSlotConfig);
	} else {
		Trace("Error in StringsEqualAction::DoExecAction, String1 not defined");
		return false;
	}
	Trace("String1: [" << string1 << "]");

	if (config.LookupPath(roaSlotConfig, "String2")) {
		Renderer::RenderOnString(string2, ctx, roaSlotConfig);
	} else {
		Trace("Error in StringsEqualAction::DoExecAction, String2 not defined");
		return false;
	}
	Trace("String2: [" << string2 << "]");

	if ( string1.IsEqual(string2) ) {
		Trace("Returning TRUE");
		return true;
	}
	Trace("Returning FALSE");
	return false;
}
