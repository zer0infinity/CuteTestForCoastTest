/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "UnTaintRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "URLUtils.h"
#include "Dbg.h"

//--- project modules used -----------------------------------------------------

//--- c-modules used -----------------------------------------------------------

//---- UnTaintRenderer ---------------------------------------------------------------
RegisterRenderer(UnTaintRenderer);

UnTaintRenderer::UnTaintRenderer(const char *name) : Renderer(name) { }

UnTaintRenderer::~UnTaintRenderer() { }

void UnTaintRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(UnTaintRenderer.RenderAll);

	String toRender(RenderToString(ctx, config["ToClean"]));
	Trace("toRender: " << toRender);
	TraceAny(config, "The config");
	String badChars(RenderToString(ctx, config["BadChars"]));
	badChars.Append("&%<>");
	Trace("badChars: " << badChars);
	String result(URLUtils::ExhaustiveUrlDecode(toRender, false));
	Trace("result ExhaustiveUrlDecode: " << result);
	result = URLUtils::ExhaustiveHTMLDecode(result);
	Trace("result ExhaustiveHTMLDecode: " << result);
	result = URLUtils::RemoveUnwantedChars(result, badChars);
	Trace("result CleanupArguments: " << result);
	reply << result;
}
