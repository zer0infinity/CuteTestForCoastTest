/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringCompareRenderer.h"

RegisterRenderer(StringCompareRenderer);

void StringCompareRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config) {
	StartTrace(StringCompareRenderer.Render);
	TraceAny(config, "config");
	ROAnything S1Config, S2Config;
	if (config.LookupPath(S1Config, "String1") && // if there are no two strings
			config.LookupPath(S2Config, "String2")) { // forget about it
		String s1;
		RenderOnString(s1, c, S1Config);
		String s2;
		RenderOnString(s2, c, S2Config);
		bool ignoreCase(config["IgnoreCase"].AsBool(0));
		if (ignoreCase) {
			s1.ToLower();
			s2.ToLower();
		}
		ROAnything conf;
		if (s1.IsEqual(s2)) { // They are equal
			Trace(s1 << " equals " << s2);
			if (config.LookupPath(conf, "Equal")) {
				Render(reply, c, conf);
			}
		} else if (config.LookupPath(conf, "Unequal")) {
			Trace(s1 << " is not equal " << s2);
			Render(reply, c, conf);
		}
	}
}
