/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _UTF8Renderer_H
#define _UTF8Renderer_H

#include "Renderer.h"

//! Converts ascii character String to UTF8 representation
/*!
 <B>Configuration:</B><PRE>
 {
 /String			Rendererspec	mandatory, String to be converted
 }
 or just
 {
 "name" "default renderer specification"
 }
 </PRE>
 */
class UTF8Renderer: public Renderer {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	UTF8Renderer(const char *name) :
		Renderer(name) {
	}
protected:
	//! Converts String into UTF8 character stream
	/*! @copydoc Renderer::RenderAll(std::ostream &, Context &, const ROAnything &) */
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif
