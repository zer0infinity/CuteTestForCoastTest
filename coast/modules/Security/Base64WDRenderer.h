/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Base64WDRenderer_H
#define _Base64WDRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_security.h"
#include "Renderer.h"

//---- Base64WDRenderer ----------------------------------------------------------
//! Coast style Base64 encoding
class EXPORTDECL_SECURITY Base64WDRenderer : public Renderer
{
public:
	//--- constructors
	Base64WDRenderer(const char *name);
	~Base64WDRenderer();

	//! Renders ?? on <I>reply </I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param ctx the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
