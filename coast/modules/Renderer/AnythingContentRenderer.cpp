/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnythingContentRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- AnythingContentRenderer ---------------------------------------------------------
RegisterRenderer(AnythingContentRenderer);

AnythingContentRenderer::AnythingContentRenderer(const char *name) : Renderer(name)
{
}

AnythingContentRenderer::~AnythingContentRenderer()
{
}

void AnythingContentRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(AnythingContentRenderer.RenderAll);
	TraceAny(config, "config");
	String whatToPrint;
	bool addXmpTags(config.IsDefined("XmpTags"));
	String cssTag(config["XmpTags"]["CSSTag"].AsString(""));
	String preTag(config["XmpTags"]["PreTag"].AsString(""));
	String postTag(config["XmpTags"]["PostTag"].AsString(""));
	bool prettyPrint(config["Pretty"].AsBool(1));
	Renderer::RenderOnString(whatToPrint, ctx, config["Input"]);

	if (addXmpTags) {
		reply << "<br><pre";
		if ( config["XmpTags"].IsDefined("CSSTag") ) {
			reply << " " << cssTag;
		}
		reply << ">";
		if ( config["XmpTags"].IsDefined("PreTag") ) {
			reply << preTag;
		}
	}
	ROAnything anyToPrint;
	if (ctx.Lookup(whatToPrint, anyToPrint)) {
		anyToPrint.PrintOn(reply, prettyPrint);
	} else {
		reply << fName << ": No Anything found for <" << whatToPrint << ">" <<
			  (addXmpTags ? "<br>" : ENDL);
	}
	Trace("Anything to print: " << whatToPrint);
	TraceAny(anyToPrint, "Content of Anything to print");
	if (addXmpTags) {
		reply << postTag << "</pre><br>";
	}
}
