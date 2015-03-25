/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LISTRENDERER_H
#define _LISTRENDERER_H

#include "Renderer.h"

//---- ListRenderer -----------------------------------------------------------
//! Renders output for all entries in a given list using a configurable renderer specification
/*!
\par Configuration
\code
{
	/ListName			Rendererspec	mandatory (can be omitted if ListData slot is used instead), resulting a String which is used to Lookup the List in the TempStore and the context
	/ListData {			Anything		optional but mandatory if ListName is omitted, inline List to use for rendering, it is ignored if ListName is also specified and is not the empty list
		...
		...
	}
	/ListHeader			Rendererspec	optional, rendered before all entries
	/ListFooter			Rendererspec	optional, rendered after after all entries
	/EntryStore			Rendererspec	optional, defaults to EntryData, resulting a String which identifies the temporary entry data while rendering
    /IndexSlot          String			optional, denotes the slotname where the index (row number) of the actual entry is stored while rendering
    /SlotnameSlot       String			optional, denotes the slotname where the slotname of the actual entry is stored while rendering
	/EntryRenderer		Rendererspec	mandatory, specification used to render each entry
	/EntryHeader		Rendererspec	optional, rendered before each entry ( depends on EHStartingEntry and EHEveryXEntries )
	/EntryHeaders {		Anything		optional, preceeds EntryHeader!, rendered before each entry after EHStartingEntry, cyling through this list
		...				Rendererspec	optional
	}
	/EHStartingEntry	long			optional, defaults to 1, ignored if there is no EntryHeader(s), list index (beginning with 1!) for which the entry header gets rendered the first time
	/EHEveryXEntries	long			optional, defaults to 1, ignored if there is no EntryHeader, EntryHeader is rendered every X entries after the EHStartingEntry
	/EntryFooter		Rendererspec	optional, rendered after each entry ( depends on EFStartingEntry and EFEveryXEntries )
	/EFStartingEntry	long			optional, defaults to 1, ignored if there is no EntryFooter, number for which the entry footer gets rendered the first time
	/EFEveryXEntries	long			optional, defaults to 1, ignored if there is no EntryFooter, EntryFooter is rendered every X entries after the EFStartingEntry
	/EFSuppressLast		slot present	optional, ignored if there is no EntryFooter, if the slot is defined EntryFooter will not be rendered after the last entry
	/Start				Rendererspec	optional, default 0, list index (zero based) with which to start rendering, useful when only a portion of the list should be used
	/End				Rendererspec	optional, default size of list, list index (zero based) with which to end rendering, useful when only a portion of the list should be used
}
\endcode

Loops over the slots of an Anything looked up in the context using the rendered ListName string. For every named or unnamed slot of this list
its content will be temporarily stored in a specified slot in the temp store. The EntryRenderer specification can then use lookups to access this
data. Additional Rendererspecs to add stylistic sugar like Headers and Footers can be added too.

Process Overview:
-# Initial config check for the EntryRenderer slot. If it fails nothing is rendered.
-# the ListRenderer looks in the TempStore for the Anything specified with /ListName. If this fails nothing is rendered.
-# If present the ListHeader is rendererd.
-# Loop over each slot in ListName
 -# The data of the actual slot in ListName are stored in TempStore.EntryStore
 -# EntryHeader is rendered depending on /EHStartingEntry and /EHEveryXEntries
 -# EntryRenderer is rendererd
 -# EntryFooter is rendered depending on /EFSuppressLast,/EFStartingEntry and /EFEveryXEntries
-# If present the ListFooter is rendererd.
*/
class ListRenderer : public Renderer
{
public:
	ListRenderer(const char *name);
	~ListRenderer();

	/*! generates output on reply driven by config using the context given
		\param reply stream to generate output on
		\param ctx Context to be used for output generation
		\param config configuration which drives the output generation */
	void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);

protected:
	/*! gets the List Data from context
		\param ctx the context the renderer runs within
		\param config the configuration of the renderer
		\param roaList a reference to the list we work on
		\return true if the list could be found in the context */
	bool GetList(Context &ctx, const ROAnything &config, ROAnything &roaList);

	/*! Get the EntryHeader to be rendered
		\param config the configuration of the renderer
		\param nr index into headers list
		\param roaEntryHeader reference to header entry to be rendered
		\param anyRenderState anything to carry information between different calls
		\return true in case an entry was found and is not Null */
	virtual bool GetEntryHeader(const ROAnything &config, long nr, ROAnything &roaEntryHeader, Anything &anyRenderState);

	/*! Get the EntryFooter to be rendered
		\param config the configuration of the renderer
		\param roaEntryFooter reference to footer entry to be rendered
		\param anyRenderState anything to carry information between different calls
		\return true in case an entry was found and is not Null */
	virtual bool GetEntryFooter(const ROAnything &config, ROAnything &roaEntryFooter, Anything &anyRenderState);

	/*! Checks if the / which EntryHeader has to be rendered
		\param ctx the context the renderer runs within
		\param config the configuration of the renderer
		\param anyRenderState anything to carry information between different calls
		\return index into headers list if multiple, 0 if only one header entry, -1 if no header has to be rendered */
	virtual long EntryHeaderNrToBeRendered(Context &ctx, const ROAnything &config, Anything &anyRenderState);

	/*! Checks if the EntryFooter has to be rendered
		\param ctx the context the renderer runs within
		\param config the configuration of the renderer
		\param anyRenderState anything to carry information between different calls
		\return true if the entry footer has to be rendered */
	virtual bool EntryFooterHasToBeRendered(Context &ctx, const ROAnything &config, Anything &anyRenderState);

	/*! renders a list header using the given configuration
		\param reply stream to generate output on
		\param ctx Context to be used for output generation
		\param listHeader renderer configuration for the header */
	virtual void RenderListHeader(std::ostream &reply, Context &ctx, const ROAnything &listHeader);

	/*! renders a list footer using the given configuration
		\param reply stream to generate output on
		\param ctx Context to be used for output generation
		\param listFooter renderer configuration for the footer */
	virtual void RenderListFooter(std::ostream &reply, Context &ctx, const ROAnything &listFooter);

	/*! renders a list entry using the given configuration
		\param reply stream to generate output on
		\param ctx Context to be used for output generation
		\param config the configuration of the renderer
		\param entryRendererConfig renderer configuration for the entry
		\param listItem current list item data as shortcut (instead of using ctx.Lookup )
		\param anyRenderState anything to carry information between different calls */
	virtual void RenderEntry(std::ostream &reply, Context &ctx, const ROAnything &config, const ROAnything &entryRendererConfig, const ROAnything &listItem, Anything &anyRenderState);

	/*! renders a entry header using the given configuration
		\param reply stream to generate output on
		\param ctx Context to be used for output generation
		\param entryHeader renderer configuration for the entry header
		\param listItem current list item data as shortcut (instead of using ctx.Lookup )
		\param anyRenderState anything to carry information between different calls */
	virtual void RenderEntryHeader(std::ostream &reply, Context &ctx, const ROAnything &entryHeader, const ROAnything &listItem, Anything &anyRenderState);

	/*! renders a entry footer using the given configuration
		\param reply stream to generate output on
		\param ctx Context to be used for output generation
		\param entryFooter renderer configuration for the entry footer
		\param listItem current list item data as shortcut (instead of using ctx.Lookup )
		\param anyRenderState anything to carry information between different calls */
	virtual void RenderEntryFooter(std::ostream &reply, Context &ctx, const ROAnything &entryFooter, const ROAnything &listItem, Anything &anyRenderState);
};

#endif		// ifndef _LISTRENDERER_H
