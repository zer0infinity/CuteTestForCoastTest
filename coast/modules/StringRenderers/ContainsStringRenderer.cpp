/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ContainsStringRenderer.h"
#include "Tracer.h"

//---- ContainsStringRenderer ---------------------------------------------------------------
RegisterRenderer(ContainsStringRenderer);

void ContainsStringRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(ContainsStringRenderer.RenderAll);
	TraceAny(config, "config");
	String strSource, strContained, strResult;
	strResult = RenderToString(ctx, config["Error"]);
	RenderOnString(strSource, ctx, config["String"]);
	RenderOnString(strContained, ctx, config["Contains"]);
	if ( strSource.Length() && strContained.Length() ) {
		long lIdx = strSource.Contains(strContained);
		Trace("contains idx: " << lIdx);
		if ( lIdx >= 0 ) {
			strResult = RenderToStringWithDefault(ctx, config["True"], Anything(lIdx));
		} else {
			strResult = RenderToStringWithDefault(ctx, config["False"], Anything(lIdx));
		}
	}
	Trace("returning [" << strResult << "]");
	reply << strResult;
}
