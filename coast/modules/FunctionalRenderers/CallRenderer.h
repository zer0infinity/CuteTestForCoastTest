/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CallRenderer_H
#define _CallRenderer_H

//---- Renderer include -------------------------------------------------
#include "Renderer.h"

//---- CallRenderer ----------------------------------------------------------
//! <B>call a renderer config with parameters</B><BR>Configuration:
//! <PRE>
//! {
//!		/Renderer		String				mandatory, name of Renderer to call with the given Parameters
//!		/Parameters {						optional
//!			/ParamName	ValueRendererspec	optional, name and its value rendererspec
//!			...
//!		}
//! }
//! </PRE>
/*!
The CallRenderer allows to reuse renderer specifications with multiple calls
of a specification on a single page with passing "local" parameters at each call.
*/
class CallRenderer : public Renderer
{
public:
	//--- constructors
	CallRenderer(const char *name);
	~CallRenderer();

	//! Renders the given renderer spec on <I>reply </I> using current local parameters
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
