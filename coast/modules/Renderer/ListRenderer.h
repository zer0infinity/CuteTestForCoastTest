/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LISTRENDERER_H
#define _LISTRENDERER_H

#include "config_renderer.h"
#include "Renderer.h"

//---- ListRenderer -----------------------------------------------------------
//! <B>Renders output for all entries in a given list using a configurable renderer specification</B>
/*!
<B>Configuration:</B><PRE>
{
	/ListData			Rendererspec		mandatory, resulting a String which is used to Lookup the List in the TempStore and the context
	/ListHeader			Rendererspec		optional, rendered before all entries
	/ListFooter			Rendererspec		optional, rendered after after all entries
	/EntryStore			Rendererspec		optional, defaults to EntryData, resulting a String which identifies the temporary entry data in the TempStore
	/EntryRenderer		Rendererspec		mandatory, specification used to render each entry
	/EntryHeader		Rendererspec		optional, rendered before each entry ( depends on EHStartingEntry and EHEveryXEntries )
	/EntryHeaders {		Anything			optional, preceeds EntryHeader!, rendered before each entry after EHStartingEntry, cyling through this list
		...				Rendererspec		optional
	}
	/EHStartingEntry	long				optional, defaults to 1, ignored if there is no EntryHeader, number for which the entry header gets rendered the first time
	/EHEveryXEntries	long				optional, defaults to 1, ignored if there is no EntryHeader, EntryHeader is rendered every X entries after the EHStartingEntry
	/EntryFooter		Rendererspec		optional, rendered after each entry ( depends on EFStartingEntry and EFEveryXEntries )
	/EFStartingEntry	long				optional, defaults to 1, ignored if there is no EntryFooter, number for which the entry footer gets rendered the first time
	/EFEveryXEntries	long				optional, defaults to 1, ignored if there is no EntryFooter, EntryFooter is rendered every X entries after the EFStartingEntry
	/EFSuppressLast		slot present		optional, ignored if there is no EntryFooter, if the slot is defined EntryFooter will not be rendered after the last entry
    /IndexSlot          String				optional, denotes the slotname where the index (row number) of the actual entry is stored in the TempStore
    /SlotnameSlot       String				optional, denotes the slotname where the slotname of the actual entry is stored in the TempStore
}
</PRE>
Loops over the slots of an Anything looked up in the context using the rendered ListData string. For every named or unnamed slot of this list
its content will be temporarily stored in a specified slot in the temp store. The EntryRenderer specification can then use lookups to access this
data. Additional Rendererspecs to add stylistic sugar like Headers and Footers can be added too.

Process Overview:
-# Initial config check for the EntryRenderer slot. If it fails nothing is rendered.
-# the ListRenderer looks in the TempStore for the Anything specified with /ListData. If this fails nothing is rendered.
-# If present the ListHeader is rendererd.
-# Loop over each slot in ListData
 -# The data of the actual slot in ListData are stored in TempStore.EntryStore
 -# EntryHeader is rendered depending on /EHStartingEntry and /EHEveryXEntries
 -# EntryRenderer is rendererd
 -# EntryFooter is rendered depending on /EFSuppressLast,/EFStartingEntry and /EFEveryXEntries
-# If present the ListFooter is rendererd.
*/
class EXPORTDECL_RENDERER ListRenderer : public Renderer
{
public:
	ListRenderer(const char *name);
	~ListRenderer();

	//! the well known Renderer main method
	void RenderAll(ostream &reply, Context &c, const ROAnything &config);

protected:
	//! Retrieves the List data and returns them in list
	//! \return false if the list retrieval fails
	/*! Maybe overwritten to fetch the list from another store
	*/
	virtual bool GetList(Context &c, const ROAnything &config, Anything &list);

	//! stores the entry data in the specified temp store slot
	/*! maybe overwritten to do additional computation and/or store the data in another store
	*/
	virtual void StoreEntryData(Context &c, const ROAnything &config, Anything &list, long index, String &entryStoreName);

	// Checks if the EntryHeader / EntryFooter should be rendererd for the actual entry index
	// ?? Should the entry data also be passed to let subclasses make the decision depending on them too ??
	virtual long EntryHeaderNrToBeRendered(Context &c, const ROAnything &config, Anything &list, long entryIndex);
	virtual bool EntryFooterHasToBeRendered(Context &c, const ROAnything &config, Anything &list, long entryIndex);

	virtual void RenderListHeader(ostream &reply, Context &c, const ROAnything &listHeader, Anything &list);
	virtual void RenderListFooter(ostream &reply, Context &c, const ROAnything &listFooter, Anything &list);
	virtual void RenderEntry(ostream &reply, Context &c, const ROAnything &config, const ROAnything &entryRendererConfig, Anything &listItem);
	virtual void RenderEntryHeader(ostream &reply, Context &c, const ROAnything &entryHeader, Anything &listItem);
	virtual void RenderEntryFooter(ostream &reply, Context &c, const ROAnything &entryFooter, Anything &listItem);
};

#endif		// ifndef _LISTRENDERER_H
