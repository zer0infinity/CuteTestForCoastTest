/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface ------------
#include "ContainsStringRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- ContainsStringRenderer ---------------------------------------------------------------
RegisterRenderer(ContainsStringRenderer);

ContainsStringRenderer::ContainsStringRenderer(const char *name)
	: Renderer(name)
{
}

ContainsStringRenderer::~ContainsStringRenderer()
{
}

void ContainsStringRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(ContainsStringRenderer.RenderAll);
	TraceAny(config, "config");
	String strSource, strContained, strResult;
	strResult = RenderToString(c, config["Error"]);
	RenderOnString(strSource, c, config["String"]);
	RenderOnString(strContained, c, config["Contains"]);
	if ( strSource.Length() && strContained.Length() ) {
		long lIdx = strSource.Contains(strContained);
		Trace("contains idx: " << lIdx);
		if ( lIdx >= 0 ) {
			strResult = RenderToStringWithDefault(c, config["True"], Anything(lIdx));
		} else {
			strResult = RenderToStringWithDefault(c, config["False"], Anything(lIdx));
		}
	}
	Trace("returning [" << strResult << "]");
	reply << strResult;
}
