/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _IncrementRenderer_H
#define _IncrementRenderer_H

#include "config_renderer.h"
#include "Renderer.h"

//---- IncrementRenderer ---------------------------------------------------------------
//! <B>This is a simple renderer used to increment a renderer Number</B>
/*!
\par Configuration
\code
{
	/IncValue		Rendererspec	mandatory, rendered string is converted to a number and incremented by one
}
\endcode
One can use
\code
/IncrementRenderer { /Lookup XYZ }
\endcode
as a short cut for
\code
/IncrementRenderer { /IncValue { /Lookup XYZ } }
\endcode

Non numeric values are converted to 0 and incremented to 1
*/
class EXPORTDECL_RENDERER IncrementRenderer : public Renderer
{
public:
	IncrementRenderer(const char *name);
	void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
