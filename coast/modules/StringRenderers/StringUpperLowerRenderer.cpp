/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface ------------
#include "StringUpperLowerRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- StringUpperLowerRenderer ---------------------------------------------------------------
RegisterRenderer(StringUpperLowerRenderer);

StringUpperLowerRenderer::StringUpperLowerRenderer(const char *name) : Renderer(name) { }

StringUpperLowerRenderer::~StringUpperLowerRenderer() { }

void StringUpperLowerRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(StringUpperLowerRenderer.RenderAll);

	TraceAny(config, "config");
	String value, mode;
	ROAnything roaSlotConfig;

	if (config.LookupPath(roaSlotConfig, "Value")) {
		RenderOnString(value, c, roaSlotConfig);
	} else {
		Trace("Error in StringUpperLowerRenderer::RenderAll, Value not defined");
		reply << String("");
		return;
	}
	Trace("Value: [" << value << "]");

	if (config.LookupPath(roaSlotConfig, "Mode")) {
		RenderOnString(mode, c, roaSlotConfig);
	} else {
		Trace("Error in StringUpperLowerRenderer::RenderAll, Mode not defined");
		reply << String("");
		return;
	}
	Trace("Mode: [" << mode << "]");

	if ( ( ! mode.IsEqual("upper") ) &&  ( ! mode.IsEqual("lower") ) ) {
		Trace("Error in StringUpperLowerRenderer::RenderAll, Mode not allowed");
		reply << String("");
		return;
	}
	if ( mode.IsEqual("upper") ) {
		value.ToUpper();
		reply << value;
	} else {
		value.ToLower();
		reply << value;
	}
}
