/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
//--- interface include --------------------------------------------------------
#include "ConditionalAction.h"
//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
//---- ConditionalAction ---------------------------------------------------------------
RegisterAction(ConditionalAction);

bool ConditionalAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace1(ConditionalAction.DoExecAction, "transition [" << transitionToken << "]");
	ROAnything roaActionSpec;
	if (!config.LookupPath(roaActionSpec, "Call")) {
		Trace("No Call specification, returning false");
		return false;
	}
	bool result = ExecAction(transitionToken, ctx, roaActionSpec);
	Trace("executing Call action was " << (result?"":"not ") << "successful, new transition [" << transitionToken << "]");
	if ( config.LookupPath(roaActionSpec, result ? "True" : "False", '\000') ) {
		TraceAny(roaActionSpec, "next action specification for transition [" << transitionToken << "]");
		return ExecAction(transitionToken, ctx, roaActionSpec);
	}
	return true;
}
