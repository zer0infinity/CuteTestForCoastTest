/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ContainsRenderer_H
#define _ContainsRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- ContainsRenderer ----------------------------------------------------------
//! <B>Conditional Renderering if a given value is contained in a List ( ListName.Contains(ValueToCheck) )</B>
/*!
\par Configuration
\code
{
	/ListName		Rendererspec		mandatory, the name of the list in which the ValueToCheck is searched
	/ValueToCheck	Rendererspec		mandatory, the value that is searched in the list.
	/True			Rendererspec		optional, is Rendered if ValueToCheck is contained in the List
	/False			Rendererspec		optional, is Rendered if ValueToCheck is not contained in the List
}
\endcode

*/
class EXPORTDECL_RENDERER ContainsRenderer : public Renderer
{
public:
	//--- constructors
	ContainsRenderer(const char *name);
	~ContainsRenderer();

	/*! Renders conditionally if ValueToCheck is contained in the List
		\param reply out - the stream where the rendered output is written on.
		\param c the context the renderer runs within.
		\param config the configuration of the renderer. */
	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
