/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SlotNameRenderer_H
#define _SlotNameRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_StringRenderers.h"
#include "Renderer.h"

//---- SlotNameRenderer ----------------------------------------------------------
//! Renderers the name of an Array-Anything slot to the given index, if any.
/*!
\par Configuration
\code
{
	/PathName	Rendererspec		mandatory, Rendered result is the path to the Anything of which the name of the slot to the given Index will be output
	/Index		Rendererspec		mandatory, Rendered result defines the index of the array entry whose slotname will be output. Will not output anything in case of an out of bound index.
}
\endcode
\par Example:
\code
/MyAny {
	/A01	First
	/B01	Second
	/C01	Last
}
{ /SlotNameRenderer {
	/PathName	MyAny
	/Index		1
} }
\endcode

Renders : "B01"
*/
class EXPORTDECL_STRINGRENDERERS SlotNameRenderer : public Renderer
{
public:
	//--- constructors
	SlotNameRenderer(const char *name);
	~SlotNameRenderer();

	//! Render the requested slotname
	/*! \param reply stream to generate output on
		\param c Context used for output generation
		\param config configuration which drives the output generation */
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif
