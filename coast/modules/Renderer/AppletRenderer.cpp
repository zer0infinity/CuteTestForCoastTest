/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AppletRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- AppletRenderer --------------------------------------------------------------
RegisterRenderer(AppletRenderer);
AppletRenderer::AppletRenderer(const char *name) : Renderer(name) {}

void AppletRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(AppletRenderer.Render);
	TraceAny(config, "config");

	ROAnything startClass;
	if (config.LookupPath(startClass, "Applet")) {

		reply << "<APPLET";

		ROAnything codeBase;
		if (config.LookupPath(codeBase, "CodeBase")) {
			reply << " CODEBASE=\"";
			Render(reply, c, codeBase);
			reply << "\"";
		}

		reply << " CODE=\"";
		Render(reply, c, startClass);
		reply << "\" ";

		PrintOptions3(reply, c, config);

		reply << ">";

		long idx = config.FindIndex("Params");
		if (idx >= 0) {
			ROAnything params = config[idx];
			for (long j = 0, sz = params.GetSize(); j < sz; ++j) {
				reply << "<PARAM name=" << params.SlotName(j) << " value=\"";
				Render(reply, c, params[j]);
				reply << "\">";
			}
		}
		reply << "</APPLET>";
	} else {
		reply << "Error in AppletRenderer: undefined start class";
	}
}
