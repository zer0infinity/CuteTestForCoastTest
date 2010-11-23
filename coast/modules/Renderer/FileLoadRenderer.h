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
#include "Renderer.h"

//---- FileLoadRenderer ----------------------------------------------------------
//! Load a file, render content
/*!
\par Configuration
\code
{
	/File2Load	Rendererspec	mandatory, name of the file 2 load
	/Quiet		Rendererspec    defaults to "no" -> error message is rendered on stream
}
\endcode

*/
class FileLoadRenderer : public Renderer
{
public:
	//--- constructors
	FileLoadRenderer(const char *name);
	~FileLoadRenderer();

	//! Renders the loaded file on <I>reply</I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param ctx the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
