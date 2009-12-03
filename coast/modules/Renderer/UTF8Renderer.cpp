/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "UTF8Renderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- UTF8Renderer ---------------------------------------------------------------
RegisterRenderer(UTF8Renderer);

UTF8Renderer::UTF8Renderer(const char *name) : Renderer(name) { }

UTF8Renderer::~UTF8Renderer() { }

void UTF8Renderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(UTF8Renderer.RenderAll);
	ROAnything lookupName;
	if ( !config.LookupPath(lookupName, "String" ) ) {
		lookupName = config[0L];
	}
	TraceAny(lookupName, "rendering content to convert");
	reply << RenderToString(c, lookupName).AsUTF8();
}
