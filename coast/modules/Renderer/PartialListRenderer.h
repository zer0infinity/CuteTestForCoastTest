/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PartialListRenderer_H
#define _PartialListRenderer_H

#include "config_renderer.h"
#include "ListRenderer.h"

//---- PartialListRenderer -----------------------------------------------------------
//! <B>Renders a list like ListRenderer but only a configurable portion of it</B>
/*!
<B>Configuration:</B><PRE>
{
	/ListData		Rendererspec	optional, resulting a String which is used to Lookup the List in the TempStore and the context
	/StartPosn		Rendererspec	mandatory, specifies starting position in ListData to be actually rendered
	/EndPosn		Rendererspec	mandatory, specifies ending position in ListData to be actually rendered
}
</PRE>
<B>See ListRenderer for additional config</B>
Loops over the slots of an Anything and stores the content in a specified slot
in the temp store. Then calls another Renderer to render the prepared data.
The user may specify additional Renderer to add stylistic sugar like Headers and Footers
*/
class EXPORTDECL_RENDERER PartialListRenderer : public ListRenderer
{
public:
	PartialListRenderer(const char *name);
	~PartialListRenderer();

protected:
	//! Retrieves the List data and returns them in list
	//! \return false if the list retrieval fails
	/*! Maybe overwritten to fetch the list from another store
	*/
	virtual bool GetList(Context &c, const ROAnything &config, Anything &list);
};

#endif		// ifndef _PartialListRenderer_H
