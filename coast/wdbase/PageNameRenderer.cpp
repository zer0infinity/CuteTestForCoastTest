/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "PageNameRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Page.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- PageNameRenderer ---------------------------------------------------------------
RegisterRenderer(PageNameRenderer);

PageNameRenderer::PageNameRenderer(const char *name) : Renderer(name) { }

PageNameRenderer::~PageNameRenderer() { }

void PageNameRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(PageNameRenderer.RenderAll);

	String pageName;
	Page *p = c.GetPage();
	if (p && p->GetName(pageName)) {
		reply << pageName;
	} else {
		Trace("Context / Page korrupt: " << (p == 0) ? (const char *)pageName : "No Page in Context");
	}
}
