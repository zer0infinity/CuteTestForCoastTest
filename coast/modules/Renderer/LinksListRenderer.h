/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LinksListRenderer_H
#define _LinksListRenderer_H

#include "config_renderer.h"
#include "ListRenderer.h"

//---- LinksListRenderer -----------------------------------------------------------
//! <B>Renders a list like ListRenderer but only a configurable portion of it</B>
/*!
<B>Configuration:</B><PRE>
{
	/ListData		Rendererspec	optional, resulting a String which is used to Lookup the List in the TempStore and the context
	/StartPosn		Rendererspec	mandatory, specifies starting position in ListData to be actually rendered
	/EndPosn		Rendererspec	mandatory, specifies ending position in ListData to be actually rendered
}
</PRE>
 * LinksListRenderer :	loops over the slots of an Anything which may be delimeted and stores the content in a specified slot
 *					in the temp store. Then calls another Renderer to render the prepared data
 *					The user may specify additional Renderer to add stylistic sugar like Headers and Footers
 *
 * Configuration : same as ListRenderer, additionally
 *		Slotname			Description									Status			Remarks
 *		/ListData			Rendererspec. resulting a String			UsedbyGetList	? Possible to locate it in other stores as well
 *							which is used to Lookup the List            may be optional in subclasses
 *							in the TempStore and the context
 *		/Increment			specifies number of entries to figure in each page...
 *							If increment is greater than list size, it is set to list size.
 *							If increment is greater than list size, it is set to list size.
 *							If increment is negative the absolute value is taken
 *
 * Process Overview:
 *		.. as ListRenderer.. but...step 2 is different
 *		2.	the LinksListRenderer looks in the TempStore for the Anything specified with /ListData. If this fails nothing is rendered.
 *		the actual list that is cloned to become the list to be rendered is part of the Any given by ListData parameter. i.e. if /ListData is
 * 		an any LIST { 0 1 2 3 4 5 6 7 8 9 10 } and /Increment is 3 the following list is returned...
 *		 {
 *			 {
 *				/Number 0
 *				/StartPosn 0
 *				/EndPosn 2
 *			  }
 *			  {
 *				/Number 1
 *				/StartPosn 3
 *				/EndPosn 5
 *			  }
 *			  {
 *				/Number 2
 *				/StartPosn 6
 *				/EndPosn 8
 *			  }
 *			  {
 *				/Number 3
 *				/StartPosn 9
 *				/EndPosn 10
 *			  }
 *		 }
*/
class EXPORTDECL_RENDERER LinksListRenderer : public ListRenderer
{
public:
	LinksListRenderer(const char *name);
	~LinksListRenderer();

protected:
	// Retrieves the List data and returns them in list
	// Returns false if the list retrieval fails
	// Maybe overwritten to fetch the list from another store
	virtual bool GetList(Context &c, const ROAnything &config, Anything &list);

	// stores the entry data in the specified temp store slot
	// maybe overwritten to do additional computation and/or store the data in another store
	// virtual void StoreEntryData(Context &c, const ROAnything &config, Anything &list, long index, String &entryStoreName);

	// Checks if the EntryHeader / EntryFooter should be rendererd for the actual entry index
	// ?? Should the entry data also be passed to let subclasses make the decision depending on them too ??
	//virtual long EntryHeaderNrToBeRendered(Context &c, const ROAnything &config, Anything &list,long entryIndex);
	//virtual bool EntryFooterHasToBeRendered(Context &c, const ROAnything &config, Anything &list,long entryIndex);
};

#endif		// ifndef _LinksListRenderer_H
