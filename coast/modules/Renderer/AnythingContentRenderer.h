/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingContentRenderer_H
#define _AnythingContentRenderer_H

#include "config_renderer.h"
#include "Renderer.h"

//---- AnythingContentRenderer -----------------------------------------------------------
//! <B>Print a Anything</B>
/*!
<B>Configuration:</B><PRE>
{
	/Input	Rendererspec		mandatory,	RendererSpec used to lookup the Anything
	/Pretty					not mandatory,	default=1,	pretty Print mode
	/XmpTags				not mandatory,	add XMP tags
	{
		/PreTag								additional formatting, goes after   <xmp> tag
		/PostTag							additional formatting, goes before </xmp> tag 														}
</PRE>
*/
class EXPORTDECL_RENDERER AnythingContentRenderer : public Renderer
{
public:
	AnythingContentRenderer(const char *name);
	~AnythingContentRenderer();

	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);

private:
};

#endif		//ifndef _AnythingContentRenderer_H
