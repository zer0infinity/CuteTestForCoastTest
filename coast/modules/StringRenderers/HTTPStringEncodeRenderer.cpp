/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPStringEncodeRenderer.h"
#include "URLUtils.h"
#include "Dbg.h"

//---- HTTPStringEncodeRenderer ---------------------------------------------------------------
RegisterRenderer(HTTPStringEncodeRenderer);

HTTPStringEncodeRenderer::HTTPStringEncodeRenderer(const char *name) : Renderer(name) { }

HTTPStringEncodeRenderer::~HTTPStringEncodeRenderer() { }

void HTTPStringEncodeRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(HTTPStringEncodeRenderer.RenderAll);
	String str(32L);
	RenderOnString(str,ctx,config["String"]);
	Trace("encoded String: <" << str << ">");
	// use MSUrlEncode function because it converts almost all reserved characters
	reply << Coast::URLUtils::MSUrlEncode(str);
}
