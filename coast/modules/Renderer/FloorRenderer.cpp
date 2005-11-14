/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "FloorRenderer.h"

//---- FloorRenderer ---------------------------------------------------------------
RegisterRenderer(FloorRenderer);

FloorRenderer::FloorRenderer(const char *name) : ComparingRenderer(name) { }

FloorRenderer::~FloorRenderer() { }

long FloorRenderer::FindSlot(String &key, const ROAnything &list)
{
	StartTrace(FloorRenderer.FindSlot);

	long i = 0;
	long sz = list.GetSize();
	while ((i < sz) && (key.Compare(list.SlotName(i)) >= 0) ) {
		++i;
	}
	return --i;
}
