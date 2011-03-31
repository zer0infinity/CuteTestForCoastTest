/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "FlowController.h"
#include "Context.h"
#include "Dbg.h"
#include "FlowControllerPrepareRequestAction.h"

//---- FlowControllerPrepareRequestAction ---------------------------------------------------------------

RegisterAction(FlowControllerPrepareRequestAction);

FlowControllerPrepareRequestAction::FlowControllerPrepareRequestAction(const char *name) : Action(name) { }

FlowControllerPrepareRequestAction::~FlowControllerPrepareRequestAction() { }

bool FlowControllerPrepareRequestAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(FlowControllerPrepareRequestAction.DoExecAction);

	String controllerName = config[0L].AsString("Invalid");
	FlowController *fc = FlowController::FindFlowController(controllerName); // build the FC object...

	if (!fc) {
		return false;
	}

	bool retVal = true;
	bool result = fc->PrepareRequest(ctx, retVal);

	Trace("Retval is " << (retVal ? "true" : "false"));
	Trace("Result is " << (result ? "true" : "false"));

	return retVal && result;
}
