/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringLengthRenderer_H
#define _StringLengthRenderer_H

#include "Renderer.h"

//! Calculates length of rendered String and places it onto output stream
/*!
 \par Configuration
 \code
 {
 /Value		Rendererspec	mandatory, string to render length of
 }
 \endcode
 */
class StringLengthRenderer: public Renderer {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	StringLengthRenderer(const char *name) :
		Renderer(name) {
	}
protected:
	/*! @copydoc Renderer::RenderAll(std::ostream &, Context &, const ROAnything &) */
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
