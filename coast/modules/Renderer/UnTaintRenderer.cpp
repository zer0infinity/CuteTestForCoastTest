/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "UnTaintRenderer.h"
#include "URLUtils.h"
#include "Tracer.h"

RegisterRenderer(UnTaintRenderer);

void UnTaintRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(UnTaintRenderer.RenderAll);
	String toRender(RenderToString(ctx, config["ToClean"]));
	Trace("toRender: " << toRender);
	TraceAny(config, "The config");
	String badChars(RenderToString(ctx, config["BadChars"]));
	badChars.Append("&%<>");
	Trace("badChars: " << badChars);
	String result(Coast::URLUtils::ExhaustiveUrlDecode(toRender, false));
	Trace("result ExhaustiveUrlDecode: " << result);
	result = Coast::URLUtils::ExhaustiveHTMLDecode(result);
	Trace("result ExhaustiveHTMLDecode: " << result);
	result = Coast::URLUtils::RemoveUnwantedChars(result, badChars);
	Trace("result CleanupArguments: " << result);
	reply << result;
}
