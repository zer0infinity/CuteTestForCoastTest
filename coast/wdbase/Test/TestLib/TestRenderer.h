/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TestRenderer_H
#define _TestRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_TestLib.h"
#include "Renderer.h"

//---- TestRenderer ----------------------------------------------------------
//! <B>Do not delete this file! It is used to create a test library which
//! is used to test library loading (AppBootLoader).</B>
/*!
nothing to configure, simply renders TestRenderer on output
*/
class EXPORTDECL_TESTLIB TestRenderer : public Renderer
{
public:
	//--- constructors
	TestRenderer(const char *name);
	~TestRenderer();

	//! Renders nothing on <I>reply</I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
