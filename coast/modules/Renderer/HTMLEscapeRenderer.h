/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLEscapeRenderer_H
#define _HTMLEscapeRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- HTMLEscapeRenderer ----------------------------------------------------------
//! Render config an HTML-escaped result string
class EXPORTDECL_RENDERER HTMLEscapeRenderer : public Renderer
{
public:
	//--- constructors
	HTMLEscapeRenderer(const char *name);
	~HTMLEscapeRenderer();

	//! Renders ?? on <I>reply </I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param ctx the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
