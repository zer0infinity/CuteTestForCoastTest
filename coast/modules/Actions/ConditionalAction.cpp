/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "ConditionalAction.h"

//---- ConditionalAction ---------------------------------------------------------------
RegisterAction(ConditionalAction);

ConditionalAction::ConditionalAction(const char *name) : Action(name) { }

ConditionalAction::~ConditionalAction() { }

bool ConditionalAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(ConditionalAction.DoExecAction);

	ROAnything call;
	if (!config.LookupPath(call, "Call")) {
		return false;
	}

	bool result = ExecAction(transitionToken, ctx, call);

	String resultToken = result ? "True" : "False";

	ROAnything nextAction;
	if (config.LookupPath(nextAction, resultToken)) {
		return ExecAction(transitionToken, ctx, nextAction);
	}

	return true;
}
