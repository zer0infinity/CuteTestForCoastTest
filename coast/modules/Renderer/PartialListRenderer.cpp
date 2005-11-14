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
#include "PartialListRenderer.h"

//---- PartialListRenderer ---------------------------------------------------------
RegisterRenderer(PartialListRenderer);

PartialListRenderer::PartialListRenderer(const char *name) : ListRenderer(name)
{
}

PartialListRenderer::~PartialListRenderer()
{
}

bool PartialListRenderer::GetList(Context &c, const ROAnything &config, Anything &list)
{
	StartTrace( PartialListRenderer.GetList);

	String inStr;
	RenderOnString(inStr, c, config["StartPosn"]);
	Anything conv = Anything( inStr );
	TraceAny(conv, "string ready for conversion to long");

	long startPosn = conv.AsLong();

	if (startPosn < 0) {
		startPosn = 0;
	}

	Anything fullList;
	if (!ListRenderer::GetList(c, config, fullList)) {
		return false;
	}
	TraceAny(fullList, "full List");

	inStr = "";
	RenderOnString(inStr, c, config["EndPosn"]);
	conv = Anything( inStr );
	TraceAny(conv, "string ready for conversion to long");
	long endPosn = conv.AsLong(fullList.GetSize());

	Trace("\nstart:" << startPosn << ",end:" << endPosn);
	if (endPosn < startPosn ) {
		return false; // false
	}

	long sz = fullList.GetSize() - 1;

	if ( sz < startPosn ) {
		return false; // false
	}
	long start = startPosn;
	long end = endPosn < sz ? endPosn : sz;

	for ( long i = start ; i <= end; ++i) {
		String slotname = fullList.SlotName(i);
		if (slotname != "") {
			list[slotname] = fullList[i];
		} else {
			list.Append(fullList[i]);
		}
	}
	TraceAny(list, "List at the End");
	return true;
}
