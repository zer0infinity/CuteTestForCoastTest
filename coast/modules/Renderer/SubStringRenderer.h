/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SubStringRenderer_H
#define _SubStringRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- SubStringRenderer ----------------------------------------------------------
//! <B>String.SubString() as Renderer</B>
/*!
<B>Configuration:</B><PRE>
{
	/String		Rendererspec	mandatory, String from which to get the substring
	/Start		Long			optional, default 0, index of first character of substring
	/Length		Long			optional, default -1, length of substring, -1 means all remaining chars
}
</PRE>
*/
class EXPORTDECL_RENDERER SubStringRenderer : public Renderer
{
public:
	//--- constructors
	SubStringRenderer(const char *name);
	~SubStringRenderer();

	//! Renders ?? on <I>reply </I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
