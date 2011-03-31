/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ContainsRenderer.h"
#include "Dbg.h"

//---- ContainsRenderer ---------------------------------------------------------------
RegisterRenderer(ContainsRenderer);

ContainsRenderer::ContainsRenderer(const char *name) : Renderer(name) { }

ContainsRenderer::~ContainsRenderer() { }

void ContainsRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(ContainsRenderer.RenderAll);

	String listDataName;
	RenderOnString(listDataName, c, config["ListName"]);
	ROAnything list = c.Lookup(listDataName);
	TraceAny(list, "The List");

	String value;
	RenderOnString(value, c, config["ValueToCheck"]);
	Trace("Value To Check: <" << value << ">");

	if (list.Contains(value)) {
		Trace("list contains " << value );
		Render(reply, c, config["True"]);
	} else {
		Trace("list does not contain " << value );
		Render(reply, c, config["False"]);
	}
}
