/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "HTMLComparer.h"

//---- HTMLComparer ----------------------------------------------------------------
HTMLComparer::HTMLComparer(Anything master, Anything slave)
	: fMaster(master), fSlave(slave), fPathStack()
{
}

HTMLComparer::~HTMLComparer()
{
}

bool HTMLComparer::Compare(String &report)
{
	StartTrace(HTMLComparer.Compare);

	if (DoCompareAny(fMaster, fSlave)) {
		return true;
	}

	report << "Difference in ";
	long sz = fPathStack.GetSize();
	TraceAny(fPathStack, "Pathstack");
	for (long i = sz - 1; i >= 0; i--) {
		report << fPathStack[i].AsString("");
		if (i > 0) {
			report << ".";
		}

	}
	return false;

}

bool HTMLComparer::DoCompareAny(Anything &master, Anything &slave)
{
	StartTrace(HTMLComparer.DoCompareAny);

	Anything::EType masterType = master.GetType();
	Anything::EType slaveType = slave.GetType();
	if (masterType != slaveType || master.GetSize() != slave.GetSize()) {
		return false;
	}

	if (masterType == Anything::eArray) {
		return DoCompareAnyArray(master, slave);
	} else {
		String masterString = master.AsString("X");
		masterString.ToLower();
		Trace("Masterstring: " << masterString);

		if (masterString != "ignore") {
//	   return master.AsString("X")==slave.AsString("U");

			if (master.AsString("X") == slave.AsString("U")) {
				return true;
			} else {
				fPathStack.Append("\n");
				fPathStack.Append(master);
				fPathStack.Append("\nMaster:");
				fPathStack.Append(slave);
				fPathStack.Append("\nDifferent sections:\nSlave :");
				return false;
			}

		}
		return true;
	}
}

bool HTMLComparer::DoCompareAnyArray(Anything &master, Anything &slave)
{
	StartTrace(HTMLComparer.DoCompareAnyArray);

	long sz = master.GetSize();
	for (long i = 0; i < sz; i++) {
		Anything masterSlot = master[i];
		String masterSlotName = master.SlotName(i);
		Anything slaveSlot = slave[i];
		String slaveSlotName = slave.SlotName(i);

		if (masterSlotName != slaveSlotName) {
			Trace("Slotnames different :" << masterSlotName
				  << "/" << slaveSlotName);
			AddToPathStack(masterSlotName, i);
			return false;
		}

		if (!DoCompareAny(masterSlot, slaveSlot )) {
			Trace("Anys different :");
			TraceAny(masterSlot, "Master slot");
			TraceAny(slaveSlot, "Slave slot");
			AddToPathStack(masterSlotName, i);

			return false;
		}
	}
	return true;
}

void HTMLComparer::AddToPathStack(String path, int index)
{
	StartTrace(HTMLComparer.AddToPathStack);

	if (path != "") {
		fPathStack.Append(path);
	} else {
		fPathStack.Append(index);
	}

}
