/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "DataAccess.h"
#include "Context.h"
#include "Renderer.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "CallDataAccessAction.h"

//---- CallDataAccessAction ---------------------------------------------------------------
RegisterAction(CallDataAccessAction);

CallDataAccessAction::CallDataAccessAction(const char *name) : Action(name) { }

CallDataAccessAction::~CallDataAccessAction() { }

bool CallDataAccessAction::DoExecAction(String &action, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(CallDataAccessAction.DoExecAction);

	ROAnything dataAccessNameSpec = config["DataAccess"];
	if (config.IsDefined("DataAccess")) {
		dataAccessNameSpec = config["DataAccess"];
	} else {
		dataAccessNameSpec = config[0L];
	}

	String dataAccessName;
	Renderer::RenderOnString(dataAccessName, ctx, dataAccessNameSpec);
	if (dataAccessName.Length() == 0) {
		return false;
	}

	Anything params = config["Parameters"].DeepClone();
	Context::PushPopEntry aEntry(ctx, "ActionParameters", params);

	Trace("DataAccess to perform : " << dataAccessName);
	DataAccess da(dataAccessName);
	return da.StdExec(ctx);
}
