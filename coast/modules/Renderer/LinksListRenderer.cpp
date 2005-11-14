/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Context.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "LinksListRenderer.h"

//---- LinksListRenderer ---------------------------------------------------------
RegisterRenderer(LinksListRenderer);

LinksListRenderer::LinksListRenderer(const char *name) : ListRenderer(name)
{
}

LinksListRenderer::~LinksListRenderer()
{
}

bool LinksListRenderer::GetList(Context &c, const ROAnything &config, Anything &list)
{
	StartTrace( LinksListRenderer.GetList);

	if ( !config.IsDefined("Increment" ) ) {
		// mandatory parameter missing
		return false;
	}

	String inStr;
	TraceAny(config, "config for LinksListRenderer");
	RenderOnString(inStr, c, config["Increment"]);
	Anything conv = Anything( inStr );
	TraceAny(conv, "string ready for conversion to long");

	long increment = conv.AsLong(10);

	if ( increment < 0 ) {
		increment = -increment;
	}

	Anything fullList;
	if (!ListRenderer::GetList(c, config, fullList)) {
		return false;
	}
	// TraceAny(fullList,"full List");
	long sz = fullList.GetSize() - 1;

	if ( (increment == 0) ) {
		increment = sz + 1;
	}

	if ( (sz == -1) ) {
		return false;
	}

	long pageNo = 0;

	for (long tempSz = sz; tempSz >= 0; tempSz = tempSz - increment) {

		Trace("tempSz:" << tempSz  );
		long startPosn = pageNo * increment;
		long endPosn = startPosn + increment - 1;

		if ( endPosn > sz ) {
			endPosn = sz;
		}
		Anything newSlot;
		newSlot["Number"] = pageNo;
		newSlot["StartPosn"] = startPosn;
		newSlot["EndPosn"] = endPosn;
		TraceAny(newSlot, "new Slot");

		list.Append(newSlot);

		++pageNo;
	}
	TraceAny(list, "List at the End");

	return true;
}

