/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TAGRENDERER_H
#define _TAGRENDERER_H

#include "config_renderer.h"
#include "Renderer.h"

//---- TagRenderer -----------------------------------------------------------
//! Renders a given HTML-Tag with the end Tag
/*!
\par Configuration
\code
{
	/Tag		String			mandatory, defining the HTML-tag without brackets(eg "PRE"), the end tag is constructed by prefixing the Tag with /
	/Options {	Anything		optional, Key-Value pair list of options rendered within the tag (refer to OptionsPrinter)
		/Key	Value			optional
		Value					optional
		...
	}
	/Content	Rendererspec	optional, render the content between start and end tag
	/NoEndTag	Something		optional, If defined, suppresses the renderering of the end tag
}
\endcode

*/
class EXPORTDECL_RENDERER TagRenderer : public Renderer
{
public:
	TagRenderer(const char *name);
	~TagRenderer();

	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif		//ifndef _TAGRENDERER_H
