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
#include "AnythingUtils.h"
#include "Renderer.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "RemoveAction.h"

//---- RemoveAction ---------------------------------------------------------------
RegisterAction(RemoveAction);

RemoveAction::RemoveAction(const char *name) : Action(name) { }

RemoveAction::~RemoveAction() { }

bool RemoveAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(RemoveAction.DoExecAction);

	String slotName;
	Renderer::RenderOnString(slotName, ctx, config["Slot"]);
	if (slotName.Length()) {
		// first of all, get the correct store
		Anything anyParent;
		String store = config["Store"].AsString("");
		char delim = config["Delim"].AsCharPtr(".")[0L];
		char indexdelim = config["IndexDelim"].AsCharPtr(":")[0L];
		Trace(String("store used [") << (store.Length() >= 0L ? (const char *)store : "TmpStore") << "]");
		anyParent = StoreFinder::FindStore(ctx, store);

		// test if the path to be deleted exists in the store, avoids creation of nonexisting slot
		Anything anySlotTest;
		if (anyParent.LookupPath(anySlotTest, slotName, delim, indexdelim)) {
			// use SlotFinders IntOperate to get the correct parent anything and the slotname/index
			// to remove from
			long slotIndex = -1L;
			if (SlotFinder::IntOperate(anyParent, slotName, slotIndex, delim, indexdelim)) {
				if (slotName.Length()) {
					Trace("removing named slot [" << slotName << "]");
					anyParent.Remove(slotName);
				} else if (slotIndex != -1L) {
					Trace("removing index slot [" << slotIndex << "]");
					anyParent.Remove(slotIndex);
				}
				return true;
			}
		} else {
			Trace("path to be deleted not found! [" << slotName << "]");
			// as we do not have to delete anything we return true anyway
			return true;
		}
	}
	return false;
}
