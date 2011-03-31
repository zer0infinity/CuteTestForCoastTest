/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BlowfishRenderer_H
#define _BlowfishRenderer_H

#include "Renderer.h"

//---- BlowfishRenderer ----------------------------------------------------------
//! Renderer for Blowfish symmetric encryption
/*!
\par Configuration
\code
{
	/Base64ArmouredKey			Rendererspec	is the input recieved by a previous Blowfish-encryption with BlowfishSecurityItem. See testcases for further details.
	/BlowfishSecurityItem		Rendererspec
}
\endcode
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
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
