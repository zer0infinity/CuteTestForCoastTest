/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _UTF8Renderer_H
#define _UTF8Renderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- UTF8Renderer ----------------------------------------------------------
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
class EXPORTDECL_RENDERER UTF8Renderer : public Renderer
{
public:
	//--- constructors
	UTF8Renderer(const char *name);
	~UTF8Renderer();

	//! Converts String into UTF8 character stream
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);

private:
	String AsUTF8(String const &strInput);

};

#endif
