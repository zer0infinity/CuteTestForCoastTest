/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface ------------
#include "CompareValuesRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- CompareValuesRenderer ---------------------------------------------------------------
RegisterRenderer(CompareValuesRenderer);

CompareValuesRenderer::CompareValuesRenderer(const char *name) : Renderer(name) { }

CompareValuesRenderer::~CompareValuesRenderer() { }

void CompareValuesRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(CompareValuesRenderer.RenderAll);

	TraceAny(config, "config");
	String value1, value2, op;
	ROAnything roaSlotConfig;

	if (config.LookupPath(roaSlotConfig, "Value1")) {
		RenderOnString(value1, c, roaSlotConfig);
	} else {
		Trace("Error in CompareValuesRenderer::RenderAll, Value1 not defined");
		Render(reply, c, config["Error"]);
		return;
	}
	Trace("Value1: [" << value1 << "]");

	if (config.LookupPath(roaSlotConfig, "Value2")) {
		RenderOnString(value2, c, roaSlotConfig);
	} else {
		Trace("Error in CompareValuesRenderer::RenderAll, Value2 not defined");
		Render(reply, c, config["Error"]);
		return;
	}
	Trace("Value2: [" << value2 << "]");

	if (config.LookupPath(roaSlotConfig, "Operator")) {
		RenderOnString(op, c, roaSlotConfig);
	} else {
		Trace("Error in CompareValuesRenderer::RenderAll, op not defined");
		Render(reply, c, config["Error"]);
		return;
	}
	Trace("Operator: [" << op << "]");

	if ( op.IsEqual(">") ) {
		if (value1.AsLong(0L) > value2.AsLong(0L)) {
			Render(reply, c, config["True"]);
			return;
		}
	} else if ( op.IsEqual("<") ) {
		if (value1.AsLong(0L) < value2.AsLong(0L)) {
			Render(reply, c, config["True"]);
			return;
		}
	} else if ( op.IsEqual(">=") ) {
		if (value1.AsLong(0L) >= value2.AsLong(0L)) {
			Render(reply, c, config["True"]);
			return;
		}
	} else if ( op.IsEqual("<=") ) {
		if (value1.AsLong(0L) <= value2.AsLong(0L)) {
			Render(reply, c, config["True"]);
			return;
		}
	} else if ( op.IsEqual("==") ) {
		if (value1.AsLong(0L) == value2.AsLong(0L)) {
			Render(reply, c, config["True"]);
			return;
		}
	} else if ( op.IsEqual("!=") ) {
		if (value1.AsLong(0L) != value2.AsLong(0L)) {
			Render(reply, c, config["True"]);
			return;
		}
	}
	Render(reply, c, config["False"]);
}
