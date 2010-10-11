/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SubStringRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- SubStringRenderer ---------------------------------------------------------------
RegisterRenderer(SubStringRenderer);

SubStringRenderer::SubStringRenderer(const char *name)
	: Renderer(name)
{
}

SubStringRenderer::~SubStringRenderer()
{
}

void SubStringRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(SubStringRenderer.RenderAll);

	long start = RenderToString(ctx, config["Start"]).AsLong(0L);
	long len   = RenderToString(ctx, config["Length"]).AsLong(-1L);
	String str;

	Renderer::RenderOnString(str, ctx, config["String"]);
	if (str.Length()) {
		String ret(str.SubString(start, len));
		Trace("SubString(" << start << "," << len << ")-->" << ret);
		reply << ret;
	}
}
