/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TagRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "OptionsPrinter.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- TagRenderer ---------------------------------------------------------
RegisterRenderer(TagRenderer);

TagRenderer::TagRenderer(const char *name) : Renderer(name)
{
}

TagRenderer::~TagRenderer()
{
}

void TagRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(TagRenderer.Render);
	TraceAny(config, "config");

	ROAnything ROtag;
	if (config.LookupPath(ROtag, "Tag")) {
		// open start tag
		String tag = ROtag.AsString("");
		reply << '<' << tag << " ";

		// check for options
		ROAnything options;
		if (config.LookupPath(options, "Options")) {
			OptionsPrinter op("dummy");
			op.RenderAll(reply, c, config);
		}
//        PrintOptions3(reply,c,config);

		// close start tag
		reply << ">\n" ;

		ROAnything content;
		if (config.LookupPath(content, "Content")) {
			// render content
			Render(reply, c, content);
		}

		// render end tag
		if (! config.IsDefined("NoEndTag")) { // suppressed if config contains slot /NoEndTag
			reply << "</" << tag << ">\n";
		}
	}
}
