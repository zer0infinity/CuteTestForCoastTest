/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Base64Renderer.h"
#include "Base64.h"
//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- Base64Renderer ---------------------------------------------------------------
RegisterRenderer(Base64Renderer);

Base64Renderer::Base64Renderer(const char *name) : Renderer(name) { }

Base64Renderer::~Base64Renderer() { }

void Base64Renderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(Base64Renderer.RenderAll);
	String clearText, b64EncodedText;
	Renderer::RenderOnString(clearText, ctx, config);
	Base64Regular(fName).DoEncode(b64EncodedText, clearText);
	reply << b64EncodedText;
}
