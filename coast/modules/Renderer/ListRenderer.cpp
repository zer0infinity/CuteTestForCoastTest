/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ListRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "AnyIterators.h"

static String ENRTY_STORE_NAME_DEFAULT("EntryData", -1, Storage::Global());
//---- ListRenderer ---------------------------------------------------------
RegisterRenderer(ListRenderer);

ListRenderer::ListRenderer(const char *name) : Renderer(name)
{
}

ListRenderer::~ListRenderer()
{
}

void ListRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(ListRenderer.RenderAll);
	TraceAny(config, "config");
	ROAnything roaList;

	// Check for mandatory configuration and presence of ListName
	if ( config.IsDefined("EntryRenderer") && ( GetList(ctx, config, roaList) || config.LookupPath(roaList, "ListData") ) ) {
		Anything anyRenderState;
		ROAnything entryRendererConfig = config["EntryRenderer"];
		ROAnything listHeader = config["ListHeader"];
		ROAnything listFooter = config["ListFooter"];
		long lListSize = roaList.GetSize(), lHeaders = -1;
		// fill in information which can be used within the following rendering methods
		anyRenderState["ListSize"] = lListSize;

		if ( lListSize <= 0 ) {
			SYSWARNING("list is empty, not rendering anything!");
			return;
		}

		ROAnything roaEntryHeaderList, roaEntryHeader;
		if ( config.LookupPath(roaEntryHeaderList, "EntryHeaders") ) {
			lHeaders = roaEntryHeaderList.GetSize();
		} else if ( config.LookupPath(roaEntryHeader, "EntryHeader") ) {
			lHeaders = 0;
		}
		anyRenderState["NumHeaders"] = lHeaders;

		// retrieve the Slotname where the entryData have to be stored
		String entryStoreName;
		ROAnything es;
		if ( config.LookupPath(es, "EntryStore") ) {
			RenderOnString(	entryStoreName, ctx, es);
		} else {
			entryStoreName = ENRTY_STORE_NAME_DEFAULT;
		}

		// Start of the rendering process ---------------------------------------------------

		// Header for the whole list
		if ( !listHeader.IsNull() ) {
			RenderListHeader(reply, ctx, listHeader);
		}

		String strIndexSlot, strSlotNameSlot, strSlotName;
		strIndexSlot = config["IndexSlot"].AsString("Index");
		strSlotNameSlot = config["SlotnameSlot"].AsString("Slotname");
		long start = Renderer::RenderToString(ctx, config["Start"]).AsLong(0L);
		if ( start < 0 ) {
			start = 0L;
		}
		if ( start >= lListSize ) {
			SYSWARNING("start index given: " << start << " is beyond list size, not rendering anything!");
			return;
		}
		long end = Renderer::RenderToString(ctx, config["End"]).AsLong(lListSize);
		if ( end > lListSize ) {
			end = lListSize;
		} else if ( end < 0 ) {
			SYSWARNING("end index given: " << end << " is negative, not rendering anything!");
			return;
		}
		// fill in information which can be used within the following rendering methods
		anyRenderState["Start"] = start;
		anyRenderState["End"] = end;

		// render entries
		ROAnything roaEntry;
		AnyExtensions::Iterator<ROAnything, ROAnything, String> aIterator(roaList);
		while ( aIterator.Next(roaEntry) ) {
			long i = aIterator.Index();
			SubTraceAny(TraceEntry, roaEntry, "data at index: " << i);
			if ( i < start ) {
				continue;
			} else if ( i > end ) {
				break;
			}
			// prepare data for rendering
			// special case of PushPopEntry, last param specifies the segment which we simulate to start with
			Context::PushPopEntry<ROAnything> aEntryData(ctx, entryStoreName, roaEntry, entryStoreName);
			Anything anyAdditionalInfo;
			anyAdditionalInfo[strIndexSlot] = ( i - start );

			if ( aIterator.SlotName(strSlotName) ) {
				anyAdditionalInfo[strSlotNameSlot] = strSlotName;
			}
			Context::PushPopEntry<Anything> aEntryDataInfo(ctx, "EntryDataInfo", anyAdditionalInfo);

			// fill in some state information which can be used within the following rendering methods to transfer some state if needed
			anyRenderState["ListIndex"] = i;
			anyRenderState["RenderIndex"] = ( i - start );
			if ( strSlotName.Length() ) {
				anyRenderState["Slotname"] = strSlotName;
			} else {
				anyRenderState.Remove("Slotname");
			}

			long nr = EntryHeaderNrToBeRendered(ctx, config, anyRenderState);
			if ( nr >= 0 ) {
				// select corresponding header entry to be rendered in case there are more than just one
				if ( GetEntryHeader(config, nr, roaEntryHeader, anyRenderState) ) {
					RenderEntryHeader(reply, ctx, roaEntryHeader, roaEntry, anyRenderState);
				}
			}

			//  render entry
			RenderEntry(reply, ctx, config, entryRendererConfig, roaEntry, anyRenderState);

			ROAnything roaEntryFooter;
			if ( EntryFooterHasToBeRendered(ctx, config, anyRenderState) && GetEntryFooter(config, roaEntryFooter, anyRenderState) ) {
				RenderEntryFooter(reply, ctx, roaEntryFooter, roaEntry, anyRenderState);
			}
		}

		// Footer for the whole list
		if ( !listFooter.IsNull() ) {
			RenderListFooter(reply, ctx, listFooter);
		}
	}
}

void ListRenderer::RenderListHeader(ostream &reply, Context &ctx, const ROAnything &listHeader)
{
	StartTrace(ListRenderer.RenderListHeader);
	Render(reply, ctx, listHeader);
}

void ListRenderer::RenderListFooter(ostream &reply, Context &ctx, const ROAnything &listFooter)
{
	StartTrace(ListRenderer.RenderListFooter);
	Render(reply, ctx, listFooter);
}

void ListRenderer::RenderEntry(ostream &reply, Context &ctx, const ROAnything &config, const ROAnything &entryRendererConfig, const ROAnything &listItem, Anything &anyRenderState)
{
	StartTrace(ListRenderer.RenderEntry);
	Render(reply, ctx, entryRendererConfig);
}

void ListRenderer::RenderEntryHeader(ostream &reply, Context &ctx, const ROAnything &entryHeader, const ROAnything &listItem, Anything &anyRenderState)
{
	StartTrace(ListRenderer.RenderEntryHeader);
	Render(reply, ctx, entryHeader);
}

void ListRenderer::RenderEntryFooter(ostream &reply, Context &ctx, const ROAnything &entryFooter, const ROAnything &listItem, Anything &anyRenderState)
{
	StartTrace(ListRenderer.RenderEntryFooter);
	Render(reply, ctx, entryFooter);
}

bool ListRenderer::GetList(Context &ctx, const ROAnything &config, ROAnything &roaList)
{
	StartTrace(ListRenderer.GetList);
	String strListDataName;
	RenderOnString(strListDataName, ctx, config["ListName"]);
	return ( strListDataName.Length() && ctx.Lookup(strListDataName, roaList) );
}

long ListRenderer::EntryHeaderNrToBeRendered(Context &ctx, const ROAnything &config, Anything &anyRenderState)
{
	StartTrace(ListRenderer.EntryHeaderNrToBeRendered);
	long render = -1, startingEntry = config["EHStartingEntry"].AsLong(1);
	long lNumHeaders = anyRenderState["NumHeaders"].AsLong();
	// these indexes are 1-based, not zero!
	long entryIndex = anyRenderState["RenderIndex"].AsLong() + 1L;
	if ( lNumHeaders > 0 ) {
		if ( entryIndex >= startingEntry ) {
			render = (entryIndex - startingEntry) % lNumHeaders;
		}
	} else if ( lNumHeaders == 0 ) {
		long everyXEntries = config["EHEveryXEntries"].AsLong(1);
		if ( ( entryIndex >= startingEntry ) && ( ( ( entryIndex - startingEntry ) % everyXEntries ) == 0 ) ) {
			render = 0;
		}
	}

	return render;
}

bool ListRenderer::EntryFooterHasToBeRendered(Context &ctx, const ROAnything &config, Anything &anyRenderState)
{
	StartTrace(ListRenderer.EntryFooterHasToBeRendered);
	bool render(false);
	long lListSize = anyRenderState["ListSize"].AsLong();
	// these indexes are 1-based, not zero!
	long entryIndex = anyRenderState["RenderIndex"].AsLong() + 1L;
	if ( config.IsDefined("EntryFooter") ) {
		long startingEntry = config["EFStartingEntry"].AsLong(1);
		long everyXEntries = config["EFEveryXEntries"].AsLong(1);

		render = ( ( entryIndex >= startingEntry ) && ( ( ( entryIndex - startingEntry ) % everyXEntries ) == 0 ) );
		// check if its the last entry
		if ( render && ( entryIndex == lListSize ) ) {
			render = !config["EFSuppressLast"].AsBool(false);
		}
	}
	return render;
}

bool ListRenderer::GetEntryHeader(const ROAnything &config, long nr, ROAnything &roaEntryHeader, Anything &anyRenderState)
{
	StartTrace(ListRenderer.GetEntryHeader);
	ROAnything roaEntryHeaderList;
	if ( config.LookupPath(roaEntryHeaderList, "EntryHeaders") ) {
		roaEntryHeader = roaEntryHeaderList[nr];
		TraceAny(roaEntryHeader, "using header entry nr: " << nr);
	} else if ( config.LookupPath(roaEntryHeaderList, "EntryHeader") && nr == 0 ) {
		roaEntryHeader = roaEntryHeaderList;
		TraceAny(roaEntryHeader, "using header");
	}
	return !roaEntryHeader.IsNull();
}

bool ListRenderer::GetEntryFooter(const ROAnything &config, ROAnything &roaEntryFooter, Anything &anyRenderState)
{
	StartTrace(ListRenderer.GetEntryFooter);
	return config.LookupPath(roaEntryFooter, "EntryFooter");
}
