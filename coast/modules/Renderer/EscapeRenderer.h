/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _EscapeRenderer_H
#define _EscapeRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- EscapeRenderer ----------------------------------------------------------
//! <B>Inserts a given escape character before the character to escape</B>
/*!
<B>Configuration:</B><PRE>
{
	/String			Rendererspec	mandatory, String to be escaped
	/EscapeChar		String			optional, default '\', defines the escape character
	/ToEscape		String			mandatory, character to escape
}
</PRE>
<B>Example:</B>
<PRE>
{ /EscapeRenderer {
	/String			"You can't escape this String's content"
	/EscapeChar		"\"
	/ToEscape		"'"
} }
</PRE>
Renders : "You can\'t escape this String\'s content"
 */
class EXPORTDECL_RENDERER EscapeRenderer : public Renderer
{
public:
	//--- constructors
	EscapeRenderer(const char *name);
	~EscapeRenderer();

	//!Sets an escape character before defined characters in a String
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
