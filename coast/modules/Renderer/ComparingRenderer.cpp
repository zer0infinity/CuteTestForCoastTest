/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ComparingRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- ComparingRenderer ---------------------------------------------------------------
ComparingRenderer::ComparingRenderer(const char *name) : Renderer(name) { }

ComparingRenderer::~ComparingRenderer() { }

void ComparingRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(ComparingRenderer.RenderAll);
	String listDataName;
	RenderOnString(listDataName, c, config["ListData"]);
	ROAnything list = c.Lookup(listDataName);
	TraceAny(list, "The List");

	String key;
	RenderOnString(key, c, config["Key"]);
	Trace("The key: <" << key << ">");

	long slotNr = FindSlot(key, list);
	if (slotNr >= 0) {
		Trace("The result: <" << list[slotNr].AsCharPtr() << ">");
		reply << list[slotNr].AsCharPtr();
	}
}
