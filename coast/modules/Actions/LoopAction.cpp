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

//--- project modules used -----------------------------------------------------
#include "Context.h"
#include "Renderer.h"

//--- interface include --------------------------------------------------------
#include "LoopAction.h"

//---- LoopAction ---------------------------------------------------------------
RegisterAction(LoopAction);

LoopAction::LoopAction(const char *name) : Action(name) { }

LoopAction::~LoopAction() { }

bool LoopAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(LoopAction.DoExecAction);

	bool breakOut = config["BreakOutOfLoop"].AsBool(0);
	long start = GetLong(ctx, config["Start"]);
	long end = GetLong(ctx, config["End"]);
	long increment = (start < end) ? 1L : -1L;

	String indexSlotName;
	Renderer::RenderOnString(indexSlotName, ctx, config["IndexSlot"]);

	Anything tmpStore = ctx.GetTmpStore();
	long stop = end + increment;
	bool ret;
	for (long i = start; i != stop; i += increment) {
		tmpStore[indexSlotName] = i;
		ret = ExecAction(transitionToken, ctx, config["Action"]);
		if ( ret == false && breakOut ) {
			return ret;
		}
	}
	return true;
}

long LoopAction::GetLong(Context &ctx, const ROAnything &config)
{
	StartTrace(LoopAction.GetLong);

	String str;
	Renderer::RenderOnString(str, ctx, config);
	long result = str.AsLong(0L);

	return result;
}
