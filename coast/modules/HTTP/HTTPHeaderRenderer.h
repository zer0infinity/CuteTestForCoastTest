/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPHeaderRenderer_H
#define _HTTPHeaderRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_http.h"
#include "Renderer.h"

//---- HTTPHeaderRenderer ----------------------------------------------------------
//! <B>Interpret an anything and render it as a MIME Header</B>
/*!
<B>Configuration:</B><PRE>
{
	/HeaderSlot		Rendererspec	[optional], or use first value
	...
}</PRE>
*/
class EXPORTDECL_HTTP HTTPHeaderRenderer : public Renderer
{
public:
	//--- constructors
	HTTPHeaderRenderer(const char *name);
	~HTTPHeaderRenderer();

	//! Renders ?? on <I>reply </I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);

private:
	void RenderHeader(ostream &reply, Context &ctx, const ROAnything &config);
	void RenderValues(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
