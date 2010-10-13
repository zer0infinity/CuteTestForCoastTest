/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "CallLDAPDAICacheAction.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "Context.h"
#include "LDAPDAICachePolicyModule.h"

//---- CallLDAPDAICacheAction ---------------------------------------------------------------
RegisterAction(CallLDAPDAICacheAction);

CallLDAPDAICacheAction::CallLDAPDAICacheAction(const char *name) : Action(name) { }

CallLDAPDAICacheAction::~CallLDAPDAICacheAction() { }

bool CallLDAPDAICacheAction::DoExecAction(String &action, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(CallLDAPDAICacheAction.DoExecAction);
	TraceAny(config, "config");

	ROAnything result;
	String key(config["Key"].AsString());
	String daName(config["Da"].AsString());
	bool ret = true;
	if (key == "*") {
		result = LDAPDAICacheGetter::GetAll(daName);
	} else {
		ret = LDAPDAICacheGetter::Get(result, daName, key);
	}
	ctx.GetTmpStore() = result.DeepClone();
	return ret;
}
