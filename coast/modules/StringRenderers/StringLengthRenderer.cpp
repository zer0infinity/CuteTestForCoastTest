/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StringLengthRenderer.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- StringLengthRenderer ---------------------------------------------------------------
RegisterRenderer(StringLengthRenderer);

StringLengthRenderer::StringLengthRenderer(const char *name) : Renderer(name) { }

StringLengthRenderer::~StringLengthRenderer() { }

void StringLengthRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(StringLengthRenderer.RenderAll);

	ROAnything roaSlotConfig;
	String strValue, strLength;
	if (config.LookupPath(roaSlotConfig, "Value")) {
		RenderOnString(strValue, ctx, roaSlotConfig);
	} else {
		Trace("Error in StringLengthRenderer::RenderAll, Value not defined");
		reply << "";
		return;
	}
	Trace("Value: [" << strValue << "]");

	reply << strValue.Length();

}
