/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _XMLEscapeRenderer_H
#define _XMLEscapeRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- XMLEscapeRenderer ----------------------------------------------------------
//! <B>Escapes &lt &gt &amp &quot &apos</B>
class EXPORTDECL_RENDERER XMLEscapeRenderer : public Renderer
{
public:
	//--- constructors
	XMLEscapeRenderer(const char *name);
	~XMLEscapeRenderer();

	//! Renders ?? on <I>reply </I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

//---- XMLEscapeRenderer ----------------------------------------------------------
//! <B>Unescapes &lt &gt &amp &quot &apos</B>
class EXPORTDECL_RENDERER XMLUnescapeRenderer : public Renderer
{
public:
	//--- constructors
	XMLUnescapeRenderer(const char *name);
	~XMLUnescapeRenderer();

	//! Renders ?? on <I>reply </I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
