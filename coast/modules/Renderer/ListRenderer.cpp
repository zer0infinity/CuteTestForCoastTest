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
#include "ListRenderer.h"

static String ENRTY_STORE_NAME_DEFAULT("EntryData", -1, Storage::Global());
//---- ListRenderer ---------------------------------------------------------
RegisterRenderer(ListRenderer);

ListRenderer::ListRenderer(const char *name) : Renderer(name)
{
}

ListRenderer::~ListRenderer()
{
}

void ListRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(ListRenderer.RenderAll);
	TraceAny(config, "config");
	Anything list;

	// Check for mandatory configuration and presence of ListData
	if (config.IsDefined("EntryRenderer") &&
		GetList(c, config, list) ) {
		// retrieve config data

		ROAnything entryRendererConfig = config["EntryRenderer"];
		ROAnything listHeader = config["ListHeader"];
		ROAnything listFooter = config["ListFooter"];
		Anything entryHeader;
		if (config.IsDefined("EntryHeaders") ) {
			entryHeader = config["EntryHeaders"].DeepClone();
		} else {
			entryHeader[0L] = config["EntryHeader"].DeepClone();
		}
		ROAnything entryFooter = config["EntryFooter"];

		// retrieve the Slotname where the entryData have to be stored
		String entryStoreName;
		ROAnything es;
		if ( config.LookupPath(es, "EntryStore") ) {
			RenderOnString(	entryStoreName, c, es);
		} // if config is defined EntryStore
		else {
			entryStoreName = ENRTY_STORE_NAME_DEFAULT;
		} // else config is defined EntryStore

		// Start of the rendering process ---------------------------------------------------

		// Header for the whole list
		if ( ! listHeader.IsNull() ) {
			RenderListHeader(reply, c, listHeader, list);
		}

		// Entries
		long sz = list.GetSize();
		for (int i = 0; i < sz; ++i) {
			// prepare data for rendering
			StoreEntryData(c, config, list, i, entryStoreName);

			long nr = EntryHeaderNrToBeRendered(c, config, list, i);
			if (nr >= 0) {
				RenderEntryHeader(reply, c, entryHeader[nr], list[i]);
			}

			//  render entry
			RenderEntry(reply, c, config, entryRendererConfig, list[i]);

			if (EntryFooterHasToBeRendered(c, config, list, i)) {
				RenderEntryFooter(reply, c, entryFooter, list[i]);
			}
		}

		// Footer for the whole list
		if ( ! listFooter.IsNull() ) {
			RenderListFooter(reply, c, listFooter, list);
		}
	}	// if initial checks are ok
}

void ListRenderer::RenderListHeader(ostream &reply, Context &c, const ROAnything &listHeader, Anything &list)
{
	StartTrace(ListRenderer.RenderListHeader);
	Render(reply, c, listHeader);
}

void ListRenderer::RenderListFooter(ostream &reply, Context &c, const ROAnything &listFooter, Anything &list)
{
	StartTrace(ListRenderer.RenderListFooter);
	Render(reply, c, listFooter);
}

void ListRenderer::RenderEntry(ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, Anything &listItem)
{
	StartTrace(ListRenderer.RenderEntry);
	Render(reply, c, entryRendererConfig);
}

void ListRenderer::RenderEntryHeader(ostream &reply, Context &c, const ROAnything &entryHeader, Anything &listItem)
{
	StartTrace(ListRenderer.RenderEntryHeader);
	Render(reply, c, entryHeader);
}

void ListRenderer::RenderEntryFooter(ostream &reply, Context &c, const ROAnything &entryFooter, Anything &listItem)
{
	StartTrace(ListRenderer.RenderEntryFooter);
	Render(reply, c, entryFooter);
}

bool ListRenderer::GetList(Context &c, const ROAnything &config, Anything &list)
{
	bool found(false);
	String listDataName;
	RenderOnString(listDataName, c, config["ListData"]);

	// Check first within TempStore - prevents us from DeepCloning
	Anything tempStore = c.GetTmpStore();
	if ( tempStore.IsDefined(listDataName) ) {
		list = tempStore[listDataName];
		found = true;
	} // if ListData is in tempStore
	else {
		ROAnything ROlist = c.Lookup(listDataName);
		if ( ! ROlist.IsNull() ) {
			list = ROlist.DeepClone();
			found = true;
		}	// if list data are found in Context
	}	// else

	return found;
}

void ListRenderer::StoreEntryData(Context &c, const ROAnything &config, Anything &list, long index, String &entryStoreName)
{
	// Just stores the entry data in TempStore
	Anything tempStore = c.GetTmpStore();
	tempStore[entryStoreName] = list[index];
	ROAnything indexslot;
	if (config.LookupPath(indexslot, "IndexSlot")) {
		tempStore[indexslot.AsString("Index")] = index;
	}
	ROAnything nameslot;
	if (config.LookupPath(nameslot, "SlotnameSlot")) {
		tempStore[nameslot.AsString("Slotname")] = list.SlotName(index);
	}
}

long ListRenderer::EntryHeaderNrToBeRendered(Context &c, const ROAnything &config, Anything &list, long entryIndex)
{
	long render = -1;
	if (config.IsDefined("EntryHeaders")) {
		++entryIndex;
		long startingEntry = config["EHStartingEntry"].AsLong(1);
		if (entryIndex >= startingEntry) {
			long sz = config["EntryHeaders"].GetSize();
			render = (entryIndex - startingEntry) % sz;
		}
	} else if (config.IsDefined("EntryHeader") ) {
		++entryIndex;
		long startingEntry = config["EHStartingEntry"].AsLong(1);
		long everyXEntries = config["EHEveryXEntries"].AsLong(1);

		if ((entryIndex >= startingEntry) && 						// not before startingEntry
			((entryIndex - startingEntry) % everyXEntries == 0)) {	// check for skipped Entries
			render = 0;
		}
	} // if EntryHeader is defined

	return render;
}

bool ListRenderer::EntryFooterHasToBeRendered(Context &c, const ROAnything &config, Anything &list, long entryIndex)
{
	bool render(false);
	if (config.IsDefined("EntryFooter") ) {
		++entryIndex;
		long startingEntry = config["EFStartingEntry"].AsLong(1);
		long everyXEntries = config["EFEveryXEntries"].AsLong(1);

		render = ((entryIndex >= startingEntry) && 							// not before startingEntry
				  ((entryIndex - startingEntry) % everyXEntries == 0));		// check for skipped Entries

		if ( render && ( entryIndex == list.GetSize() ) ) {				// if its the last entry
			render = !config.IsDefined("EFSuppressLast");					// Check for suppression
		}
	}
	return render;
}
