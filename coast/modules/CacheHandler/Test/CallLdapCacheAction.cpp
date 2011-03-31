/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "CallLdapCacheAction.h"
#include "Dbg.h"
#include "Context.h"
#include "LDAPCachePolicyModule.h"

//---- CallLdapCacheAction ---------------------------------------------------------------
RegisterAction(CallLdapCacheAction);

CallLdapCacheAction::CallLdapCacheAction(const char *name) : Action(name) { }

CallLdapCacheAction::~CallLdapCacheAction() { }

bool CallLdapCacheAction::DoExecAction(String &action, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(CallLdapCacheAction.DoExecAction);
	TraceAny(config, "config");

	ROAnything result;
	String key(config["Key"].AsString());
	String daName(config["Da"].AsString());
	bool ret = true;
	if (key == "*") {
		result = LdapCacheGetter::GetAll(daName);
	} else {
		ret = LdapCacheGetter::Get(result, daName, key);
	}
	ctx.GetTmpStore() = result.DeepClone();
	return ret;
}
