/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPStringEncodeRenderer_H
#define _HTTPStringEncodeRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_StringRenderers.h"
#include "Renderer.h"

//---- HTTPStringEncodeRenderer ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
<B>Configuration:</B><PRE>
{
	/String		Rendererspec	[mandatory], ...
	...
}</PRE>
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class EXPORTDECL_STRINGRENDERERS HTTPStringEncodeRenderer : public Renderer
{
public:
	//--- constructors
	/*! \param name defines the name of the renderer */
	HTTPStringEncodeRenderer(const char *name);
	~HTTPStringEncodeRenderer();

	/*! Renders ?? on <I>reply </I>
		\param reply the stream where the rendered output is written on.
		\param ctx the context the renderer runs within.
		\param config the configuration of the renderer.
	*/
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
