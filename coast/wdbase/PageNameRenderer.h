/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PageNameRenderer_H
#define _PageNameRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_wdbase.h"
#include "Renderer.h"

//---- PageNameRenderer ----------------------------------------------------------
//! Renders the Name of the Page in the Context
/*!
\par Configuration
\code
{
}
\endcode
or
\code
*
\endcode
*/
class EXPORTDECL_WDBASE PageNameRenderer : public Renderer
{
public:
	//--- constructors
	PageNameRenderer(const char *name);
	~PageNameRenderer();

	//!Renders the Pagename
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
