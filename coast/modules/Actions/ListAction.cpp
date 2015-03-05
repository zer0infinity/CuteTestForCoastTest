/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ListAction.h"
#include "Renderer.h"
#include "Tracer.h"
#include "AnyIterators.h"

static String ENRTYACTION_STORE_NAME_DEFAULT("EntryData", -1, coast::storage::Global());
//---- ListAction ---------------------------------------------------------------

RegisterAction(ListAction);
ListAction::ListAction(const char *name)
	: Action(name)
{
}

ListAction::~ListAction()
{
}

bool ListAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(ListAction.DoExecAction);
	TraceAny(config, "config");
	ROAnything roaList;

	// Check for mandatory configuration and presence of ListName
	if ( config.IsDefined("EntryAction") && ( GetList(ctx, config, roaList) || config.LookupPath(roaList, "ListData") ) ) {
		ROAnything entryAction = config["EntryAction"];
		long lListSize = roaList.GetSize();
		// shortcut in case the list is empty
		if ( lListSize <= 0L ) {
			SYSDEBUG("shortcut exit with true because the list is empty");
			return true;
		}
		// retrieve the Slotname where the entryData have to be stored
		String entryStoreName;
		ROAnything es;
		if ( config.LookupPath(es, "EntryStore") ) {
			Renderer::RenderOnString(entryStoreName, ctx, es);
		} else {
			entryStoreName = ENRTYACTION_STORE_NAME_DEFAULT;
		}

		String strIndexSlot, strSlotNameSlot, strSlotName;
		strIndexSlot = config["IndexSlot"].AsString("Index");
		strSlotNameSlot = config["SlotnameSlot"].AsString("Slotname");
		long start = Renderer::RenderToString(ctx, config["Start"]).AsLong(0L);
		if ( start < 0 ) {
			start = 0L;
		} else if ( start >= lListSize ) {
			SYSWARNING("start index given:" << start << "is beyond list size, not executing anything!");
			return false;
		}
		long end = Renderer::RenderToString(ctx, config["End"]).AsLong(lListSize);
		if ( end > lListSize ) {
			end = lListSize;
		} else if ( end < 0 ) {
			SYSWARNING("end index given:" << end << "is negative, not executing anything!");
			return false;
		}

		// Entries
		ROAnything roaEntry;
		AnyExtensions::Iterator<ROAnything, ROAnything, String> aIterator(roaList);
		while ( aIterator.Next(roaEntry) ) {
			long i = aIterator.Index();
			SubTraceAny(TraceEntry, roaEntry, "data at index:" << i);
			if ( i < start ) {
				continue;
			} else if ( i > end ) {
				break;
			}

			// prepare data for executing action on
			// special case of PushPopEntry, last param specifies the segment which we simulate to start with
			Context::PushPopEntry<ROAnything> aEntryData(ctx, entryStoreName, roaEntry, entryStoreName);
			Anything anyAdditionalInfo;
			anyAdditionalInfo[strIndexSlot] = ( i - start );
			if ( aIterator.SlotName(strSlotName) ) {
				anyAdditionalInfo[strSlotNameSlot] = strSlotName;
			}
			Context::PushPopEntry<Anything> aEntryDataInfo(ctx, "EntryDataInfo", anyAdditionalInfo);

			if ( !ExecEntry(transitionToken, ctx, entryAction ) ) {
				return false;
			}
		}
	}
	return true;
}

bool ListAction::ExecEntry(String &transitionToken, Context &ctx, const ROAnything &entryAction)
{
	StartTrace(ListAction.EntryAction);
	return ExecAction(transitionToken, ctx, entryAction);
}

bool ListAction::GetList(Context &ctx, const ROAnything &config, ROAnything &roaList)
{
	StartTrace(ListAction.GetList);
	String strListDataName;
	Renderer::RenderOnString(strListDataName, ctx, config["ListName"]);
	return ( strListDataName.Length() && ctx.Lookup(strListDataName, roaList) );
}
