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
#include "CompareValuesAction.h"

//---- CompareValuesAction ---------------------------------------------------------------
RegisterAction(CompareValuesAction);

CompareValuesAction::CompareValuesAction(const char *name) : Action(name) { }

CompareValuesAction::~CompareValuesAction() { }

bool CompareValuesAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(CompareValuesAction.DoExecAction);

	TraceAny(config, "config");

	String value1, value2, op;

	ROAnything roaSlotConfig;

	if (config.LookupPath(roaSlotConfig, "Value1")) {
		Renderer::RenderOnString(value1, ctx, roaSlotConfig);
	} else {
		Trace("Error in CompareValuesAction::DoExecAction, Value1 not defined");
		return false;
	}
	Trace("Value1: [" << value1 << "]");

	if (config.LookupPath(roaSlotConfig, "Value2")) {
		Renderer::RenderOnString(value2, ctx, roaSlotConfig);
	} else {
		Trace("Error in CompareValuesAction::DoExecAction, Value2 not defined");
		return false;
	}
	Trace("Value2: [" << value2 << "]");

	if (config.LookupPath(roaSlotConfig, "Operator")) {
		Renderer::RenderOnString(op, ctx, roaSlotConfig);
	} else {
		Trace("Error in CompareValuesAction::DoExecAction, Operator not defined");
		return false;
	}
	Trace("Operator: [" << op << "]");

	if ( op.IsEqual(">") ) {
		return (value1.AsLong(0L) > value2.AsLong(0L));
	} else if ( op.IsEqual("<") ) {
		return (value1.AsLong(0L) < value2.AsLong(0L));
	} else if ( op.IsEqual(">=") ) {
		return (value1.AsLong(0L) >= value2.AsLong(0L));
	} else if ( op.IsEqual("<=") ) {
		return (value1.AsLong(0L) <= value2.AsLong(0L));
	} else if ( op.IsEqual("==") ) {
		return (value1.AsLong(0L) == value2.AsLong(0L));
	} else if ( op.IsEqual("!=") ) {
		return (value1.AsLong(0L) != value2.AsLong(0L));
	}

	return false;
}
