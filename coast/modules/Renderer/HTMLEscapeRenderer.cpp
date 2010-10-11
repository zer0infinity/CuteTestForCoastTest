/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTMLEscapeRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "URLUtils.h"

//--- project modules used -----------------------------------------------------
//--- c-modules used -----------------------------------------------------------

//---- HTMLEscapeRenderer ---------------------------------------------------------------
RegisterRenderer(HTMLEscapeRenderer);

HTMLEscapeRenderer::HTMLEscapeRenderer(const char *name) : Renderer(name) { }

HTMLEscapeRenderer::~HTMLEscapeRenderer() { }

void HTMLEscapeRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(HTMLEscapeRenderer.RenderAll);

	String work;
	Renderer::RenderOnString(work, ctx, config);
	reply << URLUtils::HTMLEscape(work);
}
