/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Base64WDRenderer.h"
#include "Tracer.h"
#include "Base64.h"

//---- Base64WDRenderer ---------------------------------------------------------------
RegisterRenderer(Base64WDRenderer);

Base64WDRenderer::Base64WDRenderer(const char *name) : Renderer(name) { }

Base64WDRenderer::~Base64WDRenderer() { }

void Base64WDRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(Base64WDRenderer.RenderAll);

	String clearText, b64EncodedText;
	Renderer::RenderOnString(clearText, ctx, config);
	Base64(fName).DoEncode(b64EncodedText, clearText);
	reply << b64EncodedText;
}
