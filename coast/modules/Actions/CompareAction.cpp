/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "CompareAction.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Context.h"
#include "Dbg.h"
#include "Renderer.h"

//--- c-modules used -----------------------------------------------------------

//---- CompareAction ---------------------------------------------------------------
RegisterAction(CompareAction);

CompareAction::CompareAction(const char *name) : Action(name) { }

CompareAction::~CompareAction() { }

bool CompareLiteralValues(Context &ctx, ROAnything config)
{
	StartTrace(CompareAction.CompareLiteralValues);

	String s1, s2;
	Renderer::RenderOnString(s1, ctx, config["RenderVal1"]);
	Renderer::RenderOnString(s2, ctx, config["RenderVal2"]);
	Trace("Comparing " << s1 << " with " << s2);
	return s1.IsEqual(s2);
}

bool CompareStoreValues(Context &ctx, ROAnything config)
{
	StartTrace(CompareAction.CompareStoreValues);

	String p1, p2;
	Renderer::RenderOnString(p1, ctx, config["LookupPath1"]);
	Renderer::RenderOnString(p2, ctx, config["LookupPath2"]);
	ROAnything c1, c2;

	Trace("LookupPaths: " << p1 << " and " << p2);

	c1 = ctx.Lookup(p1);
	c2 = ctx.Lookup(p2);

	String comp = config["CompareAs"].AsString();

	if ( !comp.IsEqual("Anything") ) {
		Trace("Comparing " << c1.AsString() << " with " << c2.AsString());
	} else {
		TraceAny(c1, "Comparing this...");
		TraceAny(c2, "...with that");
	}

	if ( comp.IsEqual("Long") ) {
		return c1.AsLong() == c2.AsLong();
	}
	if ( comp.IsEqual("Double") ) {
		return c1.AsDouble() == c2.AsDouble();
	}
	if ( comp.IsEqual("Bool") ) {
		return c1.AsBool() == c2.AsBool();
	}
	if ( comp.IsEqual("Anything") ) {
		bool ret;
		String compareMsg(Anything::CompareForTestCases(c1, c2, ret));
		Trace("AnyCompareMessage: " << compareMsg);
		return ret;
	}

	// default case
	return c1.AsString().IsEqual(c2.AsString());
}

bool CompareAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(CompareAction.DoExecAction);

	if ( config.IsDefined("RenderVal1") && config.IsDefined("RenderVal2") ) {
		return CompareLiteralValues(ctx, config);
	} else if ( config.IsDefined("LookupPath1") && config.IsDefined("LookupPath2") ) {
		return CompareStoreValues(ctx, config);
	}

	Trace("Nothing compared, failing...");
	return false;
}
