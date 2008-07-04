/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NAVBARRENDERER_H
#define _NAVBARRENDERER_H

#include "config_renderer.h"
#include "Renderer.h"

//---- NavBarRenderer -----------------------------------------------------------
//! <B>Renders a multi-level navigation bar using tables</B>
/*!
\par Configuration
\code
{
}
\endcode

*/
class EXPORTDECL_RENDERER NavBarRenderer : public Renderer
{
public:
	NavBarRenderer(const char *name);
	~NavBarRenderer();

	//! the well known Renderer main method
	void RenderAll(ostream &reply, Context &c, const ROAnything &config);

protected:
	//! Retrieves the List data and returns them in list
	//! \return false if the list retrieval fails
	/*! Maybe overwritten to fetch the list from another store
	*/
	virtual void RenderLevel(ostream &reply, Context &c, const ROAnything &config, Anything &list, String &entryStoreName);
	//! Searchs the info data and returns True if found
	bool GetInfo(Context &c, const ROAnything &config, Anything &info);
};

#endif		// ifndef _NAVBARRENDERER_H
