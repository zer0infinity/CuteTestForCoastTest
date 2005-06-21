/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BlowfishRenderer_H
#define _BlowfishRenderer_H

//---- Renderer include -------------------------------------------------
#include "Renderer.h"

//---- BlowfishRenderer ----------------------------------------------------------
//! <B>Renderer for Blowfish symmetric encryption</B>
/*!
<B>Configuration:</B><PRE>
{
	/Base64ArmouredKey			Rendererspec
	/BlowfishSecurityItem		Rendererspec
}</PRE>
 Purpose_Begin
	/Base64ArmouredKey is the input recieved by a previous Blowfish-encryption with BlowfishSecurityItem.
    See testcases for further details.
  Purpose_End
*/
class BlowfishRenderer : public Renderer
{
public:
	//--- constructors
	/*! \param name defines the name of the renderer */
	BlowfishRenderer(const char *name);
	~BlowfishRenderer();

	/*! Renders ?? on <I>reply </I>
		\param reply the stream where the rendered output is written on.
		\param ctx the context the renderer runs within.
		\param config the configuration of the renderer.
	*/
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
