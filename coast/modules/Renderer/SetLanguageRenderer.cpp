/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "Context.h"
#include "Tracer.h"
#include "SetLanguageRenderer.h"

//---- SetLanguageRenderer ---------------------------------------------------------------
RegisterRenderer(SetLanguageRenderer);

SetLanguageRenderer::SetLanguageRenderer(const char *name) : Renderer(name) { }

SetLanguageRenderer::~SetLanguageRenderer() { }

void SetLanguageRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(SetLanguageRenderer.RenderAll);
	TraceAny(config, "config");

	String oldLang = c.Language();
	String tempLang;
	ROAnything LangConfig, ContentConfig;
	if ( config.LookupPath(LangConfig, "TempLang") &&
		 config.LookupPath(ContentConfig, "Content") ) {
		RenderOnString(tempLang, c, LangConfig);
		c.SetLanguage(tempLang);
		Render(reply, c, ContentConfig);
		c.SetLanguage(oldLang);
	}
}
