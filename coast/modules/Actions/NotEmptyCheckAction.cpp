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
#include "Context.h"
#include "Renderer.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "NotEmptyCheckAction.h"

//---- NotEmptyCheckAction ---------------------------------------------------------------
RegisterAction(NotEmptyCheckAction);

NotEmptyCheckAction::NotEmptyCheckAction(const char *name) : Action(name) { }

NotEmptyCheckAction::~NotEmptyCheckAction() { }

bool NotEmptyCheckAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(NotEmptyCheckAction.DoExecAction);

	ROAnything query, roaSlotConfig;
	if (config.LookupPath(roaSlotConfig, "SearchBase")) {
		String strSearchBase;
		Renderer::RenderOnString(strSearchBase, ctx, roaSlotConfig);

		if (ctx.Lookup(strSearchBase, query)) {
			Trace("found SearchBase slot");
		}
	} else {
		query = ctx.GetQuery()["fields"];
	}
	TraceAny(query, "search query is:");

	String strFieldName;
	if (config.LookupPath(roaSlotConfig, "FieldName")) {
		Renderer::RenderOnString(strFieldName, ctx, roaSlotConfig);
	}
	Trace("looking up field [" << strFieldName << "]");
	String fieldToCheck = query[strFieldName].AsString("");

	if (DoCheck(fieldToCheck, ctx, config)) {
		Trace("field is not empty");
		return true;
	}
	Trace("field is empty");

	Anything tmpStore = ctx.GetTmpStore();
	String actionName;
	GetName(actionName);
	tmpStore["Error"] = actionName;
	tmpStore["ErrorData"] = strFieldName;

	return false;
}

bool NotEmptyCheckAction::DoCheck(String &fieldToCheck, Context &ctx, const ROAnything &config)
{
	StartTrace(NotEmptyCheckAction.DoCheck);

	long sz = fieldToCheck.Length();
	for (long i = 0; i < sz; i++)	{
		if (fieldToCheck[i] != ' ') {
			return true;
		}
	}

	return false;
}
