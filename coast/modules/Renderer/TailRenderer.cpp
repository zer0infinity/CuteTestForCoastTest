/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TailRenderer.h"
#include "Tracer.h"

//---- TailRenderer ---------------------------------------------------------------

RegisterRenderer(TailRenderer);

TailRenderer::TailRenderer(const char *name) : Renderer(name) { }

TailRenderer::~TailRenderer() { }

void TailRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(TailRenderer.RenderAll);

	String theString;
	RenderOnString(theString, c, config["String"]);

	String delim = config["Delim"].AsCharPtr("=");
	long pos = theString.StrRChr(delim[0L]);
	reply << theString.SubString(pos + 1);

}
