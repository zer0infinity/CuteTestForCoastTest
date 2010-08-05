/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MultiDimNavigationRenderer_H
#define _MultiDimNavigationRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_HTMLRenderers.h"
#include "Renderer.h"

//---- MultiDimNavigationRenderer ----------------------------------------------------------
//! Preconfigured Table renderer, some things are hardcoded
/*!
\par Configuration
\code
{
	/TableWidth			Width in % of the navigation-table (optional, default 20).
	/BorderWidth		Width of the border delimiting the navigation-table (optional, default 0).
	/Cellpadding		optional, default 0 pixel
	/Cellspacing		optional, default 0 pixel
	/EltsProLine		Number of navigation-elements for each line of the navigation-table (optional, default 1).
	/NrOfLines			Number of lines of the navigation-table (optional, default 1).
	/Elt_Lx_Cy			Specification of the element at line x, column y (x, y = 0,1,2...).
						The elemnts defined in such a way are common parts used by every page (optional).
	/Elt_Lx_Cy_Cond		A context condition to check.
						If specified and equalling "1", the element is rendered.
						If specified and equalling "0", the default element is rendered.
						If specified and the context lookup returns true, the element is rendered
						If specified and the context lookup does not return true, the default is rendered
						If unspecified, true is assumed
	/Elt_Default        When specified, used for Elements that are not specified.
	/Elt_Lx_Default     Overrides default for specified line.
}
\endcode

*/
class EXPORTDECL_HTMLRENDERERS MultiDimNavigationRenderer : public Renderer
{
public:
	//--- constructors
	MultiDimNavigationRenderer(const char *name);
	~MultiDimNavigationRenderer();

	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
