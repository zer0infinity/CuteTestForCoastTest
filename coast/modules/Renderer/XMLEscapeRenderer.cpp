/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "XMLEscapeRenderer.h"
#include "Tracer.h"
#include <cstring>

static const char *entitity_map[] = {
	"<",	"&lt;",
	">",	"&gt;",
	"&",	"&amp;",
	"\"",	"&quot;",
	"'",	"&apos;",
	0
};

RegisterRenderer(XMLEscapeRenderer);

void XMLEscapeRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(XMLEscapeRenderer.RenderAll);
	String input(Renderer::RenderToString(ctx, config));
	TraceAny(config, "Config");
	Trace("input: [" << input << "]");
	for (long i = 0, sz = input.Length(); i < sz; ++i) {
		char c = input[i];
		for (int iI = 0; entitity_map[iI]; iI += 2) {
			if (c == entitity_map[iI][0]) {
				reply << entitity_map[iI + 1];
				goto found;
			}
		}
		reply << c;
		found: ;
	}
}

RegisterRenderer(XMLUnescapeRenderer);

void XMLUnescapeRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(XMLUnescapeRenderer.RenderAll);
	String input(Renderer::RenderToString(ctx, config));

	StringTokenizer tok(input, '&');
	String token;
	tok(token);
	reply << token;

	while (tok(token)) {
		for (int i = 0; entitity_map[i]; i += 2) {
			if (token.StartsWith(entitity_map[i + 1] + 1)) {
				reply << entitity_map[i] << token.SubString(strlen(entitity_map[i + 1]) - 1);
				goto found;
			}
		}
		reply << "&" << token;
		found: ;
	}
}
