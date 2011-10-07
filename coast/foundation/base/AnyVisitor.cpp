/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnyVisitor.h"

void	AnyVisitor::VisitArray(const ROAnything value, const AnyImpl *id, long index, const char *slotname)
{
	// template method for convenient forward iteration when visiting
	ArrayBefore(value, id, index, slotname);
	for (long i = 0, sz = value.GetSize(); i < sz; ++i) {
		const String &strSlotname = value.VisitSlotName(i);
		ArrayBeforeElement(i, strSlotname);
		value[i].Accept(*this, i, strSlotname);
		ArrayAfterElement(i, strSlotname);
	}
	ArrayAfter(value, id, index, slotname);
}
