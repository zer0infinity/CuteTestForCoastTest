/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "FirstNonEmptyRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- FirstNonEmptyRenderer ---------------------------------------------------------------
RegisterRenderer(FirstNonEmptyRenderer);

FirstNonEmptyRenderer::FirstNonEmptyRenderer(const char *name) : Renderer(name) { }

FirstNonEmptyRenderer::~FirstNonEmptyRenderer() { }

void FirstNonEmptyRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(FirstNonEmptyRenderer.RenderAll);
	for (long i = 0; i < config.GetSize(); i++) {
		String result;
		Renderer::RenderOnString(result, ctx, config[i]);
		if (result.Length() > 0) {
			reply << result;
			return;
		}
	}
}
