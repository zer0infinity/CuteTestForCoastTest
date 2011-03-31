/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LocalizedRenderers.h"
#include "LocalizationUtils.h"
#include "Dbg.h"

//---- StringRenderer ----------------------------------------------------------------
RegisterRenderer(StringRenderer);

StringRenderer::StringRenderer(const char *name) : Renderer(name)
{
}
void StringRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(StringRenderer.Render);
	TraceAny(config, "config");
	const char *res = LocalizationUtils::Eval(c.Language(), config);
	Trace ("res is" << res );
	reply << (res ? res : ""); // fix null ptr behavior of ostream::operator<<(const char *)
	// otherwise we would output "(null)" instead
}

//---- LanguageSwitchRenderer ----------------------------------------------------------------

RegisterRenderer(LanguageSwitchRenderer);

LanguageSwitchRenderer::LanguageSwitchRenderer(const char *name) : Renderer(name)
{
}
void LanguageSwitchRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(LanguageSwitchRenderer.Render);
	TraceAny(config, "config");

	String lang = c.Language();
	ROAnything lConfig;

	if (!config.LookupPath(lConfig, lang)) {
		config.LookupPath(lConfig, "Default");
	}
	if (!lConfig.IsNull()) {
		Render(reply, c, lConfig);
	}
}
