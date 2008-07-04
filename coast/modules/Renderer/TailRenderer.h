/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TailRenderer_H
#define _TailRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- TailRenderer ----------------------------------------------------------
//! <B>Renders the right hand side part of a string delimited by the given delimiter</B>
/*!
\par Configuration
\code
{
	/String		Rendererspec	mandatory, rendered string whose most right part after Delim will be output
	/Delim		char			optional, default =
}
\endcode

*/
class EXPORTDECL_RENDERER TailRenderer : public Renderer
{
public:
	//--- constructors
	TailRenderer(const char *name);
	~TailRenderer();

	//! Renders ?? on <I>reply </I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
