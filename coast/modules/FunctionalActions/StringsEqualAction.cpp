/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "StringsEqualAction.h"
#include "Renderer.h"
RegisterAction(StringsEqualAction);

bool StringsEqualAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(StringsEqualAction.DoExecAction);
	TraceAny(config, "config");
	String string1, string2;
	ROAnything roaSlotConfig;
	if (config.LookupPath(roaSlotConfig, "String1")) {
		Renderer::RenderOnString(string1, ctx, roaSlotConfig);
		Trace("String1: [" << string1 << "]");
	} else {
		Trace("Error in StringsEqualAction::DoExecAction, String1 not defined");
		return false;
	}
	if (config.LookupPath(roaSlotConfig, "String2")) {
		Renderer::RenderOnString(string2, ctx, roaSlotConfig);
		Trace("String2: [" << string2 << "]");
	} else {
		Trace("Error in StringsEqualAction::DoExecAction, String2 not defined");
		return false;
	}
	Trace("Returning " << string1.IsEqual(string2) ? "true" : "false");
	return string1.IsEqual(string2);
}
