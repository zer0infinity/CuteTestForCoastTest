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
//! <B>Renders the Size of an anything.</B>
/*!
<B>Configuration:</B><PRE>
{
	/ListData		Rendererspec	mandatory, Defines Anything whose size shall be rendered
	/Default		Rendererspec	optional, default if LookupName name slot was not found
}
or shorthand:
{
	Data-Rendererspec	"default renderer specification"
}</PRE>
This renderer may be handy when constructing tables. It may sometimes
be handy to know the number of columns/rows, so as to be able to generate
<i>colspan</i> or <i>rowspan</i> attributes.
*/
class EXPORTDECL_RENDERER ListSizeRenderer : public Renderer
{
public:
	//--- constructors
	ListSizeRenderer(const char *name);
	void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
