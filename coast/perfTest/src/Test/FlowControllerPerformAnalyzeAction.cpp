/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "FlowController.h"
#include "FlowControllerPerformAnalyzeAction.h"
RegisterAction(FlowControllerPerformAnalyzeAction);

bool FlowControllerPerformAnalyzeAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(FlowControllerPerformAnalyzeAction.DoExecAction);

	String controllerName = config[0L].AsString("Invalid");
	FlowController *fc = FlowController::FindFlowController(controllerName); // build the FC object...

	if (!fc) {
		return false;
	}

	bool retVal = true;
	fc->PrepareRequest(ctx, retVal);
	if (!retVal) {
		return false;
	}

	Anything tmpStore = ctx.GetTmpStore();
	tmpStore["Mapper"] = tmpStore["StoredMapper"];
	Anything result;
	retVal = fc->AnalyseReply(ctx, result);

	return retVal;
}
