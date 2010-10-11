/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ListSizeRenderer_H
#define _ListSizeRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- ListSizeRenderer ----------------------------------------------------------
//! Renders the Size of an anything.
/*!
\par Configuration
\code
{
	/ListName		Rendererspec	mandatory, Defines Anything whose size shall be rendered
	/Default		Rendererspec	optional, default if LookupName name slot was not found
}
\endcode
or shorthand:
\code
{
	Data-Rendererspec	"default renderer specification"
}
\endcode

This renderer may be handy when constructing tables. It may sometimes
be handy to know the number of columns/rows, so as to be able to generate
\em colspan or \em rowspan attributes.
*/
class EXPORTDECL_RENDERER ListSizeRenderer : public Renderer
{
public:
	//--- constructors
	ListSizeRenderer(const char *name);
	void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
