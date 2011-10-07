/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LookupRenderers.h"
#include "StringStream.h"

namespace {
	const char *gcSlotName =		"LookupName";
	const char *gcDefaultName =		"Default";
	const char *gcDelimName =		"Delim";
	const char *gcIndexDelimName = "IndexDelim";
	long const lookupNameIndex = 0L,
			defaultIndex = 1L,
			delimIndex = 2L,
			indexDelimIndex = 3L;
}
void LookupRenderer::RenderAll(std::ostream &reply, Context &context, const ROAnything &config)
{
	StartTrace(LookupRenderer.Render);
	TraceAny(config, "config");

	bool isSimpleArray = false;

	ROAnything lookupName;
	if (!config.LookupPath(lookupName, gcSlotName, '\000')) {		// use new slotname
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
	if (!config.LookupPath(dft, gcDefaultName, '\000')) {
		if ( isSimpleArray && config.GetSize() > defaultIndex ) {
			dft = config[defaultIndex];
		}
	}
	if (dft.GetType() != AnyNullType) {
		Render(reply, context, dft);
	}
}

RegisterRenderer(ContextLookupRenderer);

ROAnything ContextLookupRenderer::DoLookup(Context &context, const char *name, char delim, char indexdelim)
{
	ROAnything roaRet = context.Lookup(name, delim, indexdelim);
	StatTraceAny(ContextLookupRenderer.DoLookup, roaRet, "specification for [" << NotNull(name) << "]", Coast::Storage::Current() );
	return roaRet;
}

// lookup is exclusively done in tmpStore
RegisterRenderer(StoreLookupRenderer);

ROAnything StoreLookupRenderer::DoLookup(Context &context, const char *name, char delim, char indexdelim)
{
	ROAnything roaRet;
	((ROAnything)context.GetTmpStore()).LookupPath(roaRet, name, delim, indexdelim);
	StatTraceAny(StoreLookupRenderer.DoLookup, roaRet, "specification for [" << NotNull(name) << "]", Coast::Storage::Current() );
	return roaRet;
}

// lookup is exclusively done in query
RegisterRenderer(QueryLookupRenderer);

ROAnything QueryLookupRenderer::DoLookup(Context &context, const char *name, char delim, char indexdelim)
{
	ROAnything roaRet;
	((ROAnything)context.GetQuery()).LookupPath(roaRet, name, delim, indexdelim);
	StatTraceAny(QueryLookupRenderer.DoLookup, roaRet, "specification for [" << NotNull(name) << "]", Coast::Storage::Current() );
	return roaRet;
}
