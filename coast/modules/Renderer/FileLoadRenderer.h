/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FileLoadRenderer_H
#define _FileLoadRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- FileLoadRenderer ----------------------------------------------------------
//! <B>Load a file, render content</B>
/*!
<B>Configuration:</B><PRE>
{
	/File2Load	Rendererspec	mandatory, name of the file 2 load
}
</PRE>
*/
class EXPORTDECL_RENDERER FileLoadRenderer : public Renderer
{
public:
	//--- constructors
	FileLoadRenderer(const char *name);
	~FileLoadRenderer();

	//! Renders the loaded file on <I>reply</I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param ctx the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
