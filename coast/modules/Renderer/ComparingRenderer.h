/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ComparingRenderer_H
#define _ComparingRenderer_H

#include "Renderer.h"

//---- ComparingRenderer ----------------------------------------------------------
//! Abstract base renderer that finds a slot in a Anything according to a Key
/*!
\par Configuration
\code
{
	/ListName	Rendererspec		mandatory, Rendered result is looked up, the returned lists slots are compared
	/Key		Rendererspec		mandatory, Rendered result defines the Key to compare with.
}
\endcode

The rules that define which slot matches are coded in the method FindSlot()
*/
class ComparingRenderer : public Renderer
{
public:
	//--- constructors
	ComparingRenderer(const char *name);
	~ComparingRenderer();

	/*! Renders the content of the matching slot on <I>reply </I>
		\param reply out - the stream where the rendered output is written on.
		\param c the context the renderer runs within.
		\param config the configuration of the renderer. */
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

protected:
	/*! Renders the content of the matching slot on <I>reply </I>
		\param key the Key to compare with.
		\param list list whose slots are compared.
		\return the index of the matching slot, -1 if no match. */
	virtual long FindSlot(String &key, const ROAnything &list) = 0;
};

#endif
