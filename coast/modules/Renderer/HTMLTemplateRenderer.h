/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLTEMPLATERENDERER_H
#define _HTMLTEMPLATERENDERER_H

#include "config_renderer.h"
#include "Renderer.h"

//---- HTMLTemplateRenderer ----------------------------------------------------
//! <B>Uses HTML, file or inline, as input to render a page</B>
/*!
\par Configuration
\code
{
	/TemplateName	Rendererspec	mandatory, basename of html file; no .html extension; no path!
	/Template {		Anything		optional, but mandatory if TemplateName is omitted, TemplateName takes precedence
		"list of strings containing HTML code"
		"the list may yet contain macros: [[#wd ContextLookup foo ]]"
		"all the lines of the list are simply concatenated without "
		"adding any linefeeds etc!"
	}
	/ParserConfig {					optional, local TemplateParser options, these will override global settings defined in HTMLTemplateConfig slot, see TemplateParser for available options
		...
	}
}
\endcode
The HTML template renderer is a generic renderer that allows to
render arbitrary strings or texts. It defines a specific
macro substitution mechanism that is used to embed "calls" to
other renderers within the template text to be rendered.

The macro syntax is [[\#wd renderername rendererconfig ]], where
'[[\#wd' is the predefined prefix that causes Coast to recognize
the macro.  'renderername' may be the name of any registered renderer
(inluding alias names). The remainer of the macro - until the closing ]] -
is interpreted as an Anything specifying the /Data content of the 'config'
data that is passed as input to the respective renderer.

It is typical to use just a ContextLookupRenderer and the name of a renderer
specification that is defined in some configuration file, e.g.
[[\#wd ContextLookup aRendererspecificationname ]] with 'aRendererspecificationname'
being defined in the *.any file of the respective Page (or somewhere else in the
Context).

PS: I added a slightly improved version of parameter handling, literal
Anythings are feasable and everything is read up to the closing ]] or -->

Note that the 'rendererconfig' *MUST NOT* contain ]] and that ]] cannot be
escaped. You need to use a ContextLookupRenderer if your 'config' data
also does contain ']]'. (Or you can use the still supported
"old HTML-comment style" macro: <!-- #renderername rendererconfig -->.
Nevertheless, in that case "-->" is forbidden in 'rendererconfig'.)

There are two ways to specify the template text:

-# By denoting a file name. The file is then retrieved on demand from a language
 specific subdirectory - using the language setting of the current Context.
 (Actually it is retrieved from a special cache.. see below)
-# Specifying the text inline in the *.any file as an array of strings.

Note that HTMLTemplateRenderer manages a cache for all files/templates that
were not specified inline. The cache is built at server-startup time by
processing all files of the given HTML template directory hierarchy.
These templates are interpreted only once and are then stored in the cache in a
preprocessed intermediate format. (This improves performance as compared to
templates that are defined inline.)

HTMLTemplateRenderer uses the SysLog mechanism to record error conditions!
*/
class EXPORTDECL_RENDERER HTMLTemplateRenderer : public Renderer
{
public:
	HTMLTemplateRenderer(const char *name);

	void RenderAll(ostream &reply, Context &c, const ROAnything &config);

	static void BuildCache(const ROAnything config);
	virtual class TemplateParser *GetParser();

private:
	static ROAnything fgTemplates;
	static ROAnything fgNameMap;
	friend class HTMLCacheLoaderTest;
};

#endif
