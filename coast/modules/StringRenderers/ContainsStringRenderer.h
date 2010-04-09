/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ContainsStringRenderer_H
#define _ContainsStringRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_StringRenderers.h"
#include "Renderer.h"

//---- ContainsStringRenderer ----------------------------------------------------------
//! Search string within another string.
/*!
\par Configuration
\code
{
	/String				Rendererspec	mandatory, Rendered result is the string in which to search and replace
	/Contains 			Rendererspec	mandatory, Rendered result defines the string to be contained
	/True				Rendererspec	optional, default: position in /String [0-length), returned when contained
	/False				Rendererspec	optional, default: -1, returned when not contained
	/Error				Rendererspec	optional, default "", returned when either /String or /Contains resulted in empty string
}
\endcode
\par Example1:
\code
{ /ContainsStringRenderer {
	/String		"fooBar, gagaGugus"
	/Contains	"gaga"
} }
\endcode

Renders : "8"

\par Example2:
\code
{ /ContainsStringRenderer {
	/String		"fooBar, gagaGugus"
	/Contains	"blabla"
	/False		"Oops!"
} }
\endcode

Renders : "Oops!"
*/
class EXPORTDECL_STRINGRENDERERS ContainsStringRenderer : public Renderer
{
public:
	//--- constructors
	ContainsStringRenderer(const char *name);
	~ContainsStringRenderer();

	//! Renders the search/replace string
	/*! \param reply stream to generate output on
		\param c Context used for output generation
		\param config configuration which drives the output generation */
	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
