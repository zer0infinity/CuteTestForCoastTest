/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingContentRenderer_H
#define _AnythingContentRenderer_H

#include "Renderer.h"

//---- AnythingContentRenderer -----------------------------------------------------------
//! Render the content of an Anything
/*!
\par Configuration
\code
{
	/Input		Rendererspec	mandatory, RendererSpec used to lookup the Anything in Context
	/Pretty		long			optional, default=1, pretty Print mode
	/XmpTags					optional, add XMP tags
	{
		/CSSTag					additional style class to be placed within pre tag
		/PreTag					additional formatting, goes after   <xmp> tag
		/PostTag				additional formatting, goes before </xmp> tag
	}
}
\endcode
*/
class AnythingContentRenderer : public Renderer
{
public:
	AnythingContentRenderer(const char *name);
	~AnythingContentRenderer();

	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif		//ifndef _AnythingContentRenderer_H
