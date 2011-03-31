/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "Context.h"
#include "Dbg.h"
#include "TestAction.h"

//---- TestAction ----------------------------------------------------------------
RegisterAction(TestAction);

TestAction::TestAction(const char *name) : Action(name) { }

TestAction::~TestAction() { }

bool TestAction::DoAction(String &name, Context &ctx)
{
	StartTrace(TestAction.DoAction);

	Anything tmpStore = ctx.GetTmpStore();
	tmpStore[fName] = name;

	if ( name == "ChangeMeTrue" ) {
		name = "Changed";
		return true;
	}
	if ( name == "ChangeMeFalse" ) {
		name = "Changed";
		return false;
	}

	if ( name == "ReturnFalse" ) {
		return false;
	}
	return true;

}

//---- ConfiguredTestAction ----------------------------------------------------------------
RegisterAction(ConfiguredTestAction);

bool ConfiguredTestAction::DoExecAction(String &name, Context &ctx, const ROAnything &config)
{
	StartTrace(ConfiguredTestAction.DoExecAction);

	Anything tmpStore = ctx.GetTmpStore();
	tmpStore[fName] = name;

	if (config.IsDefined("ActionToken") ) {
		name = config["ActionToken"].AsString("x");
	}
	return config["RetValue"].AsBool(0);

}
