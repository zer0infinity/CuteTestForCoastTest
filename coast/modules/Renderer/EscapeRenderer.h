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
#include "Renderer.h"

//---- EscapeRenderer ----------------------------------------------------------
//! Inserts a given escape character before the character(s) to escape
/*!
\par Configuration
\code
{
	/String			Rendererspec	mandatory, String to be escaped
	/EscapeChar		String			optional, default '\', defines the escape character
	/ToEscape		String			mandatory, characters to escape
}
\endcode
\par Example
\code
{ /EscapeRenderer {
	/String			"You can't escape this String's content"
	/EscapeChar		"\\"
	/ToEscape		"'"
} }
\endcode

Renders : "You can\'t escape this String\'s content"
 */
class EscapeRenderer : public Renderer
{
public:
	//--- constructors
	EscapeRenderer(const char *name);
	~EscapeRenderer();

	//!Sets an escape character before defined characters in a String
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif
