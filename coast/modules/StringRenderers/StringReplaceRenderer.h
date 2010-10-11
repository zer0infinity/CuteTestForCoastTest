/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringReplaceRenderer_H
#define _StringReplaceRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_StringRenderers.h"
#include "Renderer.h"

//---- StringReplaceRenderer ----------------------------------------------------------
//! Replaces strings within a specified string.
/*!
\par Configuration
\code
{
	/String				Rendererspec	mandatory, Rendered result is the string in which to search and replace
	/ReplaceConfig {					mandatory, Rendered result defines the index of the array entry whose slotname will be output. Will not output anything in case of an out of bound index.
		/Search		Replace
		...
	}
}
\endcode
\par Example1:
\code
{ /StringReplaceRenderer {
	/String		"fooBar, gagaGugus"
	/ReplaceConfig {
		/foo	""
		/gaga	""
	}
} }
\endcode

Renders : "Bar, Gugus"

\par Example2:
\code
{ /StringReplaceRenderer {
	/String		"fooBar, gagaGugus"
	/ReplaceConfig {
		/foo	"my"
		/gaga	"your"
	}
} }
\endcode

Renders : "myBar, yourGugus"
*/
class EXPORTDECL_STRINGRENDERERS StringReplaceRenderer : public Renderer
{
public:
	//--- constructors
	StringReplaceRenderer(const char *name);
	~StringReplaceRenderer();

	//! Renders the search/replace string
	/*! \param reply stream to generate output on
		\param c Context used for output generation
		\param config configuration which drives the output generation */
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif
