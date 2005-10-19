/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface ------------
#include "CreateAnythingFromStringAction.h"

//--- standard modules used ----------------------------------------------------
#include "AnythingUtils.h"
#include "SysLog.h"
#include "Renderer.h"
#include "Dbg.h"

//---- CreateAnythingFromStringAction ---------------------------------------------------------------
RegisterAction(CreateAnythingFromStringAction);

CreateAnythingFromStringAction::CreateAnythingFromStringAction(const char *name) : Action(name) { }

CreateAnythingFromStringAction::~CreateAnythingFromStringAction() { }

bool CreateAnythingFromStringAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(CreateAnythingFromStringAction.DoExecAction);
	TraceAny(config, "config:");

	ROAnything roString;
	String string;
	if (config.LookupPath(roString, "String")) {
		Renderer::RenderOnString(string, ctx, roString);
	} else {
		SysLog::Warning("CreateAnythingFromStringAction::DoExecAction: String slot not defined in config!");
		return false;
	}
	Trace("resulting string before creating the any:[" << string << "]");

	Anything newAny;
	if (string.Length()) {
		IStringStream is(string);
		newAny.Import(is);
	}
	TraceAny(newAny, "newAny:");

	ROAnything destConfig;
	if (!config.LookupPath(destConfig, "Destination")) {
		SysLog::Warning("CreateAnythingFromStringAction::DoExecAction: Destination slot not defined in config!");
		return false;
	}

	StorePutter::Operate(newAny, ctx, destConfig);

	return true;
}
