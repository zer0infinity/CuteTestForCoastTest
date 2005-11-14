/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ListAction.h"

//--- standard modules used ----------------------------------------------------
#include "Renderer.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

static String ENRTYACTION_STORE_NAME_DEFAULT("EntryData", -1, Storage::Global());
//---- ListAction ---------------------------------------------------------------

RegisterAction(ListAction);

ListAction::ListAction(const char *name) : Action(name) { }

ListAction::~ListAction() { }

bool ListAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(ListAction.DoExecAction);

	TraceAny(config, "config");
	Anything list;

	// Check for mandatory configuration and presence of ListData
	if (config.IsDefined("EntryAction") && GetList(ctx, config, list) ) {
		// retrieve the Slotname where the entryData have to be stored
		String entryStoreName;
		ROAnything es;
		if ( config.LookupPath(es, "EntryStore") ) {
			Renderer::RenderOnString(entryStoreName, ctx, es);
		} else {
			entryStoreName = ENRTYACTION_STORE_NAME_DEFAULT;
		}

		bool hasIndex = false;
		ROAnything indexslot;
		String indexSlotname;
		if (config.LookupPath(indexslot, "IndexSlot")) {
			hasIndex = true;
			indexSlotname = indexslot.AsString("Index");
		}

		bool hasSlotname = false;
		ROAnything nameslot;
		String slotnameSlotname;
		if (config.LookupPath(nameslot, "SlotnameSlot")) {
			hasSlotname = true;
			slotnameSlotname = nameslot.AsString("Slotname");
		}

		// Entries
		Anything tempStore = ctx.GetTmpStore();
		ROAnything entryAction = config["EntryAction"];
		long sz = list.GetSize();
		for (int i = 0; i < sz; ++i) {
			tempStore[entryStoreName] = list[i];
			if (hasSlotname) {
				tempStore[slotnameSlotname] = list.SlotName(i);
			}
			if (hasIndex) {
				tempStore[indexSlotname] = i;
			}
			if ( !ExecEntry(transitionToken, ctx, config, entryAction, list[i], list.SlotName(i), i) ) {
				return false;
			}
		}
	}
	return true;
}

bool ListAction::ExecEntry(String &transitionToken, Context &ctx, const ROAnything &config, const ROAnything &entryAction, Anything &anyListItem, const String &strSlotname, const long &lIndex)
{
	StartTrace(ListAction.EntryAction);
	return ExecAction(transitionToken, ctx, entryAction);
}

bool ListAction::GetList(Context &c, const ROAnything &config, Anything &list)
{
	// Copy pasted from ListRenderer
	StartTrace(ListAction.GetList);

	bool found(false);
	String listDataName;
	Renderer::RenderOnString(listDataName, c, config["ListData"]);

	// Check first within TempStore - prevents us from DeepCloning
	Anything tempStore = c.GetTmpStore();
	if ( tempStore.IsDefined(listDataName) ) {
		list = tempStore[listDataName];
		found = true;
	} else {
		ROAnything ROlist = c.Lookup(listDataName);
		if ( ! ROlist.IsNull() ) {
			list = ROlist.DeepClone();
			found = true;
		}
	}
	return found;
}
