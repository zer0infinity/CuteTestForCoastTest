/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPStringEncodeRenderer.h"

//--- project modules used -----------------------------------------------------
#include "URLUtils.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- HTTPStringEncodeRenderer ---------------------------------------------------------------
RegisterRenderer(HTTPStringEncodeRenderer);

HTTPStringEncodeRenderer::HTTPStringEncodeRenderer(const char *name) : Renderer(name) { }

HTTPStringEncodeRenderer::~HTTPStringEncodeRenderer() { }

void HTTPStringEncodeRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(HTTPStringEncodeRenderer.RenderAll);
	String str(32L);
	RenderOnString(str,ctx,config["String"]);
	Trace("encoded String: <" << str << ">");
	// use MSUrlEncode function because it converts almost all reserved characters
	reply << URLUtils::MSUrlEncode(str);
}
