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
#include "config_renderer.h"
#include "Renderer.h"

//---- CallRenderer ----------------------------------------------------------
//! Call a renderer config with parameters (lazy evaluation)
/*!
\par Configuration
\code
{
	/Renderer		String		mandatory, rendererspec to call, lookuped in context
	/Parameters {	Anything	optional, slots or parameters to pass to the called renderer
		/Key		Value
		...
	}
}
\endcode
or
\code
{
	<renderer name>		Name of Renderer to be rendered
	<arg 1>				Argument made available as $1
	<arg 2>				Argument made available as $2
	....
}
\endcode

The CallRenderer allows to reuse renderer specifications with multiple calls
of a specification on a single page with passing "local" parameters at each call.
*/
class EXPORTDECL_RENDERER CallRenderer : public Renderer
{
protected:
	virtual Anything DoGetParameters(Context &ctx, const ROAnything &config);
	virtual Anything DoGetPositionalParameters(Context &ctx, const ROAnything &config);
	ROAnything IntGetCallee(Context &ctx, ROAnything callee);
public:
	//--- constructors
	CallRenderer(const char *name);
	~CallRenderer();

	/*! Renders the given renderer spec on <I>reply </I> using current local parameters
		\param reply out - the stream where the rendered output is written on.
		\param ctx the context the renderer runs within.
		\param config the configuration of the renderer. */
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};
//---- EagerCallRenderer ----------------------------------------------------------
//! call a renderer config with rendered parameters (eager evaluation)
/*!
\par Configuration
\code
{
	/Render			String		mandatory, rendererspec to call, lookuped in context
	/Parameters {	Anything	optional, slots or parameters to pass to the called renderer
		/Key		ValueRendererSpec	value rendered instead of literal
		...
	}
}
\endcode

The EagerCallRenderer allows to reuse renderer specifications with multiple calls
of a specification on a single page with passing "local" parameters at each call.
*/
class EXPORTDECL_RENDERER EagerCallRenderer : public CallRenderer
{
protected:
	virtual Anything DoGetParameters(Context &ctx, const ROAnything &config);
	virtual Anything DoGetPositionalParameters(Context &ctx, const ROAnything &config);
public:
	//--- constructors
	EagerCallRenderer(const char *name);
	~EagerCallRenderer();
};
#endif
