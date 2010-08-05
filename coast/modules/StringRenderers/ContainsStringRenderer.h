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
 * @section cosr1 Renderer configuration
\code
{
	/String
	/Contains
	/True
	/False
	/Error
}
\endcode
 * @par \c String
 * Mandatory\n
 * Rendererspec resulting in a string value in which to search for rendered /Contains value
 *
 * @par \c Contains
 * Mandatory\n
 * Rendererspec resulting in a string to be contained within rendered /String value
 *
 * @par \c True
 * Optional, default: start position of rendered /Contains string within rendered /String, [0..length)\n
 * Rendererspec resulting in a string value to be rendered when contained
 *
 * @par \c False
 * Optional, default: -1\n
 * Rendererspec resulting in a string value to be rendered when not contained
 *
 * @par \c Error
 * Optional, default: ""\n
 * Rendererspec resulting in a string value to be rendered when either /String or /Contains rendering resulted in an empty string
 *
 * @par Example1:
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
	ContainsStringRenderer();
public:
	/*! Default constructor for NamedObject
	 * @param name defines the name of the Renderer */
	ContainsStringRenderer(const char *name);
	~ContainsStringRenderer();

	/*! Renders the corresponding result based on a String::Contains() check
	 * @param reply stream to generate output on
	 * @param c Context used for output generation
	 * @param config configuration which drives the output generation
	 */
	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
