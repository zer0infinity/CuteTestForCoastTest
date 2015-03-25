/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "Context.h"
#include "Renderer.h"
#include "Tracer.h"
#include "CallDataAccessAction.h"
#include "LDAPDAIDataAccessTestAction.h"

//---- LDAPDAIDataAccessTestAction ---------------------------------------------------------------
RegisterAction(LDAPDAIDataAccessTestAction);

LDAPDAIDataAccessTestAction::LDAPDAIDataAccessTestAction(const char *name) : CallDataAccessAction(name) { }

LDAPDAIDataAccessTestAction::~LDAPDAIDataAccessTestAction() { }

bool LDAPDAIDataAccessTestAction::DoExecAction(String &action, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(LDAPDAIDataAccessTestAction.DoExecAction);
	return CallDataAccessAction::DoExecAction(action, ctx, config);
}
