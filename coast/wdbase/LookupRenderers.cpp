/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ----------------------------------------------------------------
#include "LookupRenderers.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "Dbg.h"

static const char *gcSlotName =		"LookupName";
static const char *gcDelimName =		"Delim";
static const char *gcIndexDelimName = "IndexDelim";

//---- LookupRenderer ----------------------------------------------------------------
LookupRenderer::LookupRenderer(const char *name) : Renderer(name)
{
}

void LookupRenderer::RenderAll(ostream &reply, Context &context, const ROAnything &config)
{
	StartTrace(LookupRenderer.Render);
	TraceAny(config, "config");

	bool isSimpleArray = false;

	ROAnything lookupName;
	if (!config.LookupPath(lookupName, gcSlotName, '\000')) {		// use new slotname
		isSimpleArray = true;
		lookupName = config[0L];
	}

	ROAnything delim;
	if (!config.LookupPath(delim, gcDelimName, '\000')) {
		if ( isSimpleArray && config.GetSize() > 2 ) {
			delim = config[2L];
		}
	}

	ROAnything indexdelim;
	if (!config.LookupPath(indexdelim, gcIndexDelimName, '\000')) {
		if ( isSimpleArray && config.GetSize() > 3 ) {
			indexdelim = config[3L];
		}
	}

	// lookup data and use it as a renderer specification
	if ( !lookupName.IsNull()) {		// check if lookupName is defined somehow
		// render lookupname first
		String lookup;
		RenderOnString(lookup, context, lookupName);
		Trace("looking up: " << lookup);

		ROAnything data = DoLookup(context, lookup, delim.AsCharPtr(".")[0L], indexdelim.AsCharPtr(":")[0L]);
		TraceAny(data, "found: ");

		if (data.GetType() != AnyNullType) {
			Render(reply, context, data);
			return;
		}
	} else {
		// handle error: lookup name is not a string
		String error("LookupRenderer::RenderAll: empty lookup name for config ");
		{
			OStringStream os(error);
			config.PrintOn(os, false);	// append Anything to ease debugging
		}
		SystemLog::Error(error);
		return;
	}

	// no data found: render the default
	ROAnything dft;
	if (!config.LookupPath(dft, "Default", '\000')) {
		if ( isSimpleArray && config.GetSize() > 1 ) {
			dft = config[1L];
		}
	}
	if (dft.GetType() != AnyNullType) {
		Render(reply, context, dft);
	}
}

//---- ContextLookupRenderer ----------------------------------------------------------------
RegisterRenderer(ContextLookupRenderer);

ContextLookupRenderer::ContextLookupRenderer(const char *name)
	: LookupRenderer(name)
{
}

ROAnything ContextLookupRenderer::DoLookup(Context &context, const char *name, char delim, char indexdelim)
{
	ROAnything roaRet = context.Lookup(name, delim, indexdelim);
	StatTraceAny(ContextLookupRenderer.DoLookup, roaRet, "specification for [" << NotNull(name) << "]", Storage::Current() );
	return roaRet;
}

//---- StoreLookupRenderer ----------------------------------------------------------------
// lookup is exclusively done in tmpStore
RegisterRenderer(StoreLookupRenderer);

StoreLookupRenderer::StoreLookupRenderer(const char *name) : LookupRenderer(name)
{
}

ROAnything StoreLookupRenderer::DoLookup(Context &context, const char *name, char delim, char indexdelim)
{
	ROAnything roaRet;
	((ROAnything)context.GetTmpStore()).LookupPath(roaRet, name, delim, indexdelim);
	StatTraceAny(StoreLookupRenderer.DoLookup, roaRet, "specification for [" << NotNull(name) << "]", Storage::Current() );
	return roaRet;
}

//---- QueryLookupRenderer ----------------------------------------------------------------
// lookup is exclusively done in query
RegisterRenderer(QueryLookupRenderer);

QueryLookupRenderer::QueryLookupRenderer(const char *name) : LookupRenderer(name)
{
}

ROAnything QueryLookupRenderer::DoLookup(Context &context, const char *name, char delim, char indexdelim)
{
	ROAnything roaRet;
	((ROAnything)context.GetQuery()).LookupPath(roaRet, name, delim, indexdelim);
	StatTraceAny(QueryLookupRenderer.DoLookup, roaRet, "specification for [" << NotNull(name) << "]", Storage::Current() );
	return roaRet;
}
