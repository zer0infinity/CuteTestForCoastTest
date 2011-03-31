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

//---- StringLengthRenderer ----------------------------------------------------------
//! really brief class description
/*!
\par Configuration
\code
{
	/String		Rendererspec	mandatory, string to render lenght of
}
\endcode
*/
class StringLengthRenderer : public Renderer
{
public:
	//--- constructors
	/*! \param name defines the name of the renderer */
	StringLengthRenderer(const char *name);
	~StringLengthRenderer();

	/*! Renders ?? on <I>reply </I>
		\param reply the stream where the rendered output is written on.
		\param ctx the context the renderer runs within.
		\param config the configuration of the renderer.
	*/
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
