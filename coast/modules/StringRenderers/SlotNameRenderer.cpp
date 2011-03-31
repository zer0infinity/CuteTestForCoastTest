/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SlotNameRenderer.h"
#include "Dbg.h"

//---- SlotNameRenderer ---------------------------------------------------------------
RegisterRenderer(SlotNameRenderer);

SlotNameRenderer::SlotNameRenderer(const char *name) : Renderer(name) { }

SlotNameRenderer::~SlotNameRenderer() { }

void SlotNameRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(SlotNameRenderer.RenderAll);

	TraceAny(config, "config");

	String value, index;
	ROAnything roaSlotConfig;

	if ( config.LookupPath(roaSlotConfig, "PathName") ) {
		RenderOnString(value, c, roaSlotConfig);
	} else {
		Trace("Error in SlotNameRenderer::RenderAll, PathName not defined");
		return;
	}
	Trace("PathName: [" << value << "]");
	ROAnything roAnyTemp = c.Lookup(value);

	if ( config.LookupPath(roaSlotConfig, "Index") ) {
		RenderOnString(index, c, roaSlotConfig);
	} else {
		Trace("Error in SlotNameRenderer::RenderAll, Index not defined");
		return;
	}
	Trace("Index: [" << index << "]");

	long i = index.AsLong(-1);
	if ( i >= 0 ) {
		Trace("SlotName: [" << roAnyTemp.SlotName( i ) << "]");
		reply << roAnyTemp.SlotName( i );
	}
}
