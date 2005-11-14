/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "CeilingRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- CeilingRenderer ---------------------------------------------------------------
RegisterRenderer(CeilingRenderer);

CeilingRenderer::CeilingRenderer(const char *name) : ComparingRenderer(name) { }

CeilingRenderer::~CeilingRenderer() { }

long CeilingRenderer::FindSlot(String &key, const ROAnything &list)
{
	StartTrace(CeilingRenderer.FindSlot);

	long i = 0;
	long sz = list.GetSize();
	while ((i < sz) && (key.Compare(list.SlotName(i)) > 0) ) {
		++i;
	}

	return i < sz ? i : -1;
}
