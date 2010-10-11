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
//! Renders the first non-empty slot of the configuration Anything onto the stream
/*! @section FirstNonEmptyRendererDescription FirstNonEmptyRenderer Description
 * The main principle is to render the content of the first indexed configuration slot if its Renderer specification
 * results in a non empty String.
 * @subsection FirstNonEmptyRendererConfiguration FirstNonEmptyRenderer Configuration
\code
{
	/NamedSlot		Rendererspec	optional, gets output if the rendered content is not empty
	"unnamed slot"	Rendererspec	optional, gets output if the rendered content is not empty
	{ some complicated /ContextLookupRenderer whatever  configuration }
	...
}
\endcode
 * @subsection FirstNonEmptyRendererExample FirstNonEmptyRenderer Example
\code
/FirstNonEmptyRenderer {
	""
	"this slot is not empty"
	{ "what " "is" " going " "on " "here" }
	/CheckThis	{ /Lookup Now }
}
\endcode
 * In this example the second slot ["this slot is not empty"] will be output because the first one is empty.
*/
class EXPORTDECL_RENDERER FirstNonEmptyRenderer : public Renderer
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	FirstNonEmptyRenderer(const char *name);
	~FirstNonEmptyRenderer();

	//! Renders first non empty configuration entry as output
	/*! @copydetails Renderer::RenderAll(std::ostream &, Context &, const ROAnything &) */
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
