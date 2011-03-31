/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPHeaderRenderer_H
#define _HTTPHeaderRenderer_H

#include "Renderer.h"

//---- HTTPHeaderRenderer ----------------------------------------------------------
//! Interpret an anything and render it as a MIME Header
/*!
\par Configuration
\code
{
	/HeaderSlot		Rendererspec	[optional], or use first value
	...
}
\endcode

*/
class HTTPHeaderRenderer : public Renderer
{
public:
	//--- constructors
	HTTPHeaderRenderer(const char *name);
	~HTTPHeaderRenderer();

	//! Renders ?? on <I>reply </I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);

private:
	void RenderHeader(std::ostream &reply, Context &ctx, const ROAnything &config);
	void RenderValues(std::ostream &reply, Context &ctx, const ROAnything &config);
	void RenderMultipleLineHeaderField(std::ostream &reply, Context &ctx, const String &slot, const ROAnything &config);

};

#endif
