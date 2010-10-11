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
//! URL-Encode given string
/*!
 * @section htser1 Renderer configuration
\code
{
	/String
}
\endcode
 * @par \c String
 * Mandatory\n
 * String value which gets URL encoded
 */
class EXPORTDECL_STRINGRENDERERS HTTPStringEncodeRenderer : public Renderer
{
	HTTPStringEncodeRenderer();
public:
	/*! Default constructor for NamedObject
	 * @param name defines the name of the Renderer */
	HTTPStringEncodeRenderer(const char *name);
	~HTTPStringEncodeRenderer();

	/*! Renders URL encoded string onto stream
	 * @param reply the stream where the rendered output is written on.
	 * @param ctx the context the renderer runs within.
	 * @param config the configuration of the renderer.
	 */
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
