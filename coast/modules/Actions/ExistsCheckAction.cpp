/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ExistsCheckAction.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Context.h"
#include "Renderer.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- ExistsCheckAction ---------------------------------------------------------------
RegisterAction(ExistsCheckAction);

ExistsCheckAction::ExistsCheckAction(const char *name) : Action(name) { }

ExistsCheckAction::~ExistsCheckAction() { }

bool ExistsCheckAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(ExistsCheckAction.DoExecAction);

	String path;
	Renderer::RenderOnString(path, ctx, config);
	ROAnything collector;

	bool ret = ctx.Lookup(path, collector);
	Trace(path << " exists: " << (ret ? "true" : "false"));
	return ret;
}
