/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TransitionDispatchAction.h"

//--- standard modules used ----------------------------------------------------
#include "Renderer.h"
#include "SysLog.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- TransitionDispatchAction ---------------------------------------------------------------
RegisterAction(TransitionDispatchAction);

TransitionDispatchAction::TransitionDispatchAction(const char *name) : Action(name) { }

TransitionDispatchAction::~TransitionDispatchAction() { }

bool TransitionDispatchAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(TransitionDispatchAction.DoExecAction);

	if ( transitionToken.Length() ) {
		ROAnything nextActionConfig = ctx.Lookup(transitionToken);
		TraceAny(nextActionConfig, "nextActionConfig");
		if ( !nextActionConfig.IsNull() ) {
			return Action::ExecAction(transitionToken, ctx, nextActionConfig);
		} else {
			SYSWARNING("transition config is null, returning false!");
		}
	} else {
		SYSWARNING("transitionToken empty, returning false!");
	}
	return false;
}
