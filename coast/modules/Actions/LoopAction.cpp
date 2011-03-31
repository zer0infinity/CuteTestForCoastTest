/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LoopAction.h"
#include "Renderer.h"
#include "Dbg.h"

//---- LoopAction ---------------------------------------------------------------
RegisterAction(LoopAction);

LoopAction::LoopAction(const char *name)
	: Action(name)
{
}

LoopAction::~LoopAction()
{
}

bool LoopAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(LoopAction.DoExecAction);

	bool breakOut = config["BreakOutOnFailure"].AsBool(false);
	long start = Renderer::RenderToString(ctx, config["Start"]).AsLong(0L);
	long end = Renderer::RenderToString(ctx, config["End"]).AsLong(0L);
	long increment = (start < end) ? 1L : -1L;

	String strIndexSlot = Renderer::RenderToStringWithDefault(ctx, config["IndexSlot"], "Index");

	long stop = end + increment;
	bool ret;
	for (long i = start; i != stop; i += increment) {
		Anything anyAdditionalInfo;
		anyAdditionalInfo[strIndexSlot] = i;
		Context::PushPopEntry<Anything> aEntryDataInfo(ctx, "EntryDataInfo", anyAdditionalInfo);

		ret = ExecAction(transitionToken, ctx, config["Action"]);
		if ( ret == false && breakOut ) {
			return ret;
		}
	}
	return true;
}
