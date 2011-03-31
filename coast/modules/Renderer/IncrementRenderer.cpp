/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "IncrementRenderer.h"
#include "Dbg.h"

//---- IncrementRenderer --------------------------------------------------------------
RegisterRenderer(IncrementRenderer);
IncrementRenderer::IncrementRenderer(const char *name) : Renderer(name) {}

void IncrementRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(IncrementRenderer.Render);
	TraceAny(config, "config");

	ROAnything len;
	if (config.IsDefined("IncValue")) {
		len = config["IncValue"];
	} else {
		len = config;
	}

	if (!len.IsNull()) {
		String str;
		TraceAny(len, "IncValue to be rendered is :");
		RenderOnString(str, c, len);
		Trace("IncValue rendered is :" << str);

		Anything conv = Anything( str );
		TraceAny(conv, "string ready for conversion to long");
		long val = conv.AsLong();
		Trace("IncValue as long  is :" << val);
		++val;
		Trace("IncValue as long incremented is :" << val);
		Anything conv2 = Anything( val );
		TraceAny(conv2, "number ready for conversion to String");
		String valString = conv2.AsString();

		reply << valString;
	} else {
		reply << "Error in IncrementRenderer: undefined IncValue slot";
	}
}
