/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Context.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "LoopTestAction.h"

//---- LoopTestAction ---------------------------------------------------------------
RegisterAction(LoopTestAction);

LoopTestAction::LoopTestAction(const char *name) : Action(name) { }

LoopTestAction::~LoopTestAction() { }

bool LoopTestAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(LoopTestAction.DoExecAction);

	Anything tmpStore = ctx.GetTmpStore();
	tmpStore["LoopResult"].Append(ctx.Lookup("LoopIndex", 999L));

	return true;
}
RegisterAction(FailLoopTestAction);

FailLoopTestAction::FailLoopTestAction(const char *name) : Action(name) { }

FailLoopTestAction::~FailLoopTestAction() { }

bool FailLoopTestAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(FailLoopTestAction.DoExecAction);

	Anything tmpStore = ctx.GetTmpStore();
	long loopIndex(ctx.Lookup("LoopIndex", 999L));
	tmpStore["LoopResult"].Append(loopIndex);
	return loopIndex % 4;
}
