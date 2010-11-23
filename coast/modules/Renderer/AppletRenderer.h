/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _APPLETRENDERER_H
#define _APPLETRENDERER_H

#include "Renderer.h"

//---- AppletRenderer ---------------------------------------------------------------
//! Renderer used to embed Java Applets into an HTML page
/*!
\par Configuration
\code
{
	/CodeBase	Rendererspec	mandatory, The location of the java code relative to your web servers document root.
	/Applet						mandatory, Specifies the complete name of the Java Applet class to be used.
	/Options	{...}			optional, key-value pairs that are rendered into the tag
	/Params		{...}			optional, key-value pairs given to the applet
}
\endcode

The renderer allows to specify the applet to be used, its basic layout, and the parameters passed to the applet.
Example:
Specification of an AppletRenderer using the Java applet class 'PieApplet' from the package 'CH.ifa.toolkit'.
The code base is looked up from the context at slot 'CodeBase' (e.g. 'CodeBase' might be globally defined
once in Config.any). The size of the applet is defined to be 350 by 200 pixels.
Two parameters 'X1' and 'X2' are passed to the applet: 'X1' is defined to be the
text literal '10' whereas 'X2' is obtained by looking up 'foo' in the context.
\code
/AppletRenderer {
	/CodeBase	{ /ContextLookupRenderer CodeBase }
	/Applet		"CH.ifa.toolkit.PieApplet.class"
	/Options {
		/WIDTH	"350"
		/HEIGHT	"200"
	}
	/Params {
		/X1	"10"
		/X2 { /ContextLookupRenderer "foo" }
	}
}
\endcode
*/
class AppletRenderer : public Renderer
{
public:
	//!standard constructor
	//! \param name object name for registry
	AppletRenderer(const char *name);

	//!overridden rendering method
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif
