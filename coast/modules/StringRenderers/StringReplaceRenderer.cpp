/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringReplaceRenderer.h"
#include "Dbg.h"

//---- StringReplaceRenderer ---------------------------------------------------------------
RegisterRenderer(StringReplaceRenderer);

StringReplaceRenderer::StringReplaceRenderer(const char *name)
	: Renderer(name)
{
}

StringReplaceRenderer::~StringReplaceRenderer()
{
}

void StringReplaceRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(StringReplaceRenderer.RenderAll);

	TraceAny(config, "config");

	String strDestination, strSource;
	RenderOnString(strSource, c, config["String"]);
	ROAnything roaMapAny;
	if ( config.LookupPath(roaMapAny, "ReplaceConfig") ) {
		TraceAny(roaMapAny, " config");
		String strTmpSource = strSource;
		for ( long cfgIdx = 0L, lSize = roaMapAny.GetSize(); cfgIdx < lSize; ++cfgIdx ) {
			strDestination.Trim(0);
			String sString = roaMapAny.SlotName(cfgIdx);
			String rString = roaMapAny[sString].AsString();
			//TODO: prevent loop for each replacestring -> RegExpRenderer
			long startIdx = 0L;
			// returns -1 if there no sString can be found
			while ( ( startIdx = strTmpSource.Contains(sString) ) >= 0L ) {
				String tmpString = strTmpSource.SubString(0, startIdx);
				Trace("search string [" << sString << "] at index:" << startIdx << " segment up to sString [" << tmpString << "]");
				strDestination.Append(tmpString);
				strDestination.Append(rString);
				strTmpSource.TrimFront(startIdx + sString.Length());
				Trace("remaining inputstring [" << strTmpSource << "]");
				Trace("Destination so far [" << strDestination << "]");
			}
			Trace("appending remainder [" << strTmpSource << "]");
			strDestination.Append(strTmpSource);
			strTmpSource = strDestination;
		}
	} else {
		strDestination = strSource;
	}
	Trace("destination to append [" << strDestination << "]");
	reply << strDestination;
}
