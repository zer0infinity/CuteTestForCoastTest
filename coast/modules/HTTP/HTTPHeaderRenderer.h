/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPHeaderRenderer_H
#define _HTTPHeaderRenderer_H

#include "Renderer.h"

//! Interpret an anything and render it as a MIME Header
/*!
 \par Configuration
 \code
{
	/HeaderSlot		Rendererspec	[optional], or use first value
	...
}
\endcode
*/
class HTTPHeaderRenderer: public Renderer {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	HTTPHeaderRenderer(const char *name) :
		Renderer(name) {
	}
protected:
	/*! @copydoc Renderer::RenderAll(std::ostream &, Context &, const ROAnything &) */
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
