/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FirstNonEmptyRenderer_H
#define _FirstNonEmptyRenderer_H

#include "config_renderer.h"
#include "Renderer.h"

//---- FirstNonEmptyRenderer ----------------------------------------------------------
//! <B>Renders the first non-empty slot of the configuration Anything</B>
/*!
\par Configuration
\code
{
	/NamedSlot		Rendererspec	optional, gets output if the rendered content is not empty
	"unnamed slot"	Rendererspec	optional, gets output if the rendered content is not empty
	...
}
\endcode
\par Example:
\code
/FirstNonEmptyRenderer {
	""
	"this slot is not empty"
	{ "what " "is" " going " "on " "here" }
	/CheckThis	{ /Lookup Now }
}
\endcode

In this example the second slot ["this slot is not empty"] will be output because the first one is empty.
*/
class EXPORTDECL_RENDERER FirstNonEmptyRenderer : public Renderer
{
public:
	//--- constructors
	FirstNonEmptyRenderer(const char *name);
	~FirstNonEmptyRenderer();

	//! Renders on <I>reply</I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param ctx the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
