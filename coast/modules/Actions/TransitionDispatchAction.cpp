/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "TransitionDispatchAction.h"
#include "Context.h"
#include "SystemLog.h"
#include "Dbg.h"
//---- TransitionDispatchAction ---------------------------------------------------------------
RegisterAction(TransitionDispatchAction);

bool TransitionDispatchAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(TransitionDispatchAction.DoExecAction);
	if (transitionToken.Length()) {
		ROAnything nextActionConfig = ctx.Lookup(transitionToken);
		TraceAny(nextActionConfig, "nextActionConfig");
		if (!nextActionConfig.IsNull()) {
			return Action::ExecAction(transitionToken, ctx, nextActionConfig);
		} else {
			SYSWARNING("transition config is null, returning false!");
		}
	} else {
		SYSWARNING("transitionToken empty, returning false!");
	}
	return false;
}
