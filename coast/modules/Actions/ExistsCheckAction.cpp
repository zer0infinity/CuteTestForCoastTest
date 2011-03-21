/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "ExistsCheckAction.h"
#include "Context.h"
#include "Renderer.h"
#include "Dbg.h"
//---- ExistsCheckAction ---------------------------------------------------------------
RegisterAction(ExistsCheckAction);

namespace {
	const char *gcSlotName = "LookupName";
	const char *gcDelimName = "Delim";
	const char *gcIndexDelimName = "IndexDelim";
	long const lookupNameIndex = 0L,
			delimIndex = 1L,
			indexDelimIndex = 2L;
}

bool ExistsCheckAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(ExistsCheckAction.DoExecAction);

	bool isSimpleArray = false;

	ROAnything lookupName;
	if (!config.LookupPath(lookupName, gcSlotName, '\000')) {
		isSimpleArray = true;
		lookupName = config[lookupNameIndex];
	}

	ROAnything delim;
	if (!config.LookupPath(delim, gcDelimName, '\000')) {
		if ( isSimpleArray && config.GetSize() > delimIndex ) {
			delim = config[delimIndex];
		}
	}

	ROAnything indexdelim;
	if (!config.LookupPath(indexdelim, gcIndexDelimName, '\000')) {
		if ( isSimpleArray && config.GetSize() > indexDelimIndex ) {
			indexdelim = config[indexDelimIndex];
		}
	}

	String lookup;
	Renderer::RenderOnString(lookup, ctx, lookupName);
	Trace("looking up: " << lookup);

	bool ret = ctx.Lookup(lookup, lookupName, delim.AsCharPtr(".")[0L], indexdelim.AsCharPtr(":")[0L]);

	Trace(lookup << " exists: " << (ret ? "true" : "false"));
	return ret;
}
