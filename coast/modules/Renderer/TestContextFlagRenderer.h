/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TestContextFlagRenderer_H
#define _TestContextFlagRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- TestContextFlagRenderer ----------------------------------------------------------
//! <B>Test for existence and truth of a boolean in context</B>
/*!
<B>Configuration:</B><PRE>
{
	/FlagName		Rendererspec	mandatory, Path of the flag in Context
	/WhenSet		Rendererspec	optional, gets evaluated when flag exists and is true
	/WhenNotSet		Rendererspec	optional, gets evaluated when flag does not exist or is false
}
</PRE>
Is equivalent  /SwitchRenderer { /ContextLookupName <FlagName> /Case { /1 <IsSet> } /Default <NotSet> }
*/
class EXPORTDECL_RENDERER TestContextFlagRenderer : public Renderer
{
public:
	//--- constructors
	TestContextFlagRenderer(const char *name);
	~TestContextFlagRenderer();

	//! Renders ?? on <I>reply </I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
