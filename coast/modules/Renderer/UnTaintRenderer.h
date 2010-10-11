/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _UnTaintRenderer_H
#define _UnTaintRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- UnTaintRenderer ----------------------------------------------------------
//! Load a file, render content
/*!
\par Configuration
\code
{
	/UnTaintRenderer
	{
		/ToClean	Rendererspec	mandatory, content to untaint
		/BadChars	Rendererspec	optional, characters you want to remove from /ToClean (default is "<>")
	}
}
\endcode
This renderer unescapes the following escape sequences until there is nothing left to unescape:\n
URL-escaping: 	\%ff and \%uffff \n
HTML-escaping:	\&\#d - \&\#dddd and \&\#xf - \&\#xffff \n

Escaped values > xFF are considered invalid (they might be potentially harmful unicode chars) and discarded.
The characters given in /BadChars are removed from the unescaped string. Under any circumstances, \%\&\<\> are removed.
This renderer may be used whenever input from a client (user) is rendered into a html page.
*/
class EXPORTDECL_RENDERER UnTaintRenderer : public Renderer
{
public:
	//--- constructors
	UnTaintRenderer(const char *name);
	~UnTaintRenderer();

	//! URL unescapes a string an cleans it from dangerous characters.
	//! \param reply out - the stream where the rendered output is written on.
	//! \param ctx the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
