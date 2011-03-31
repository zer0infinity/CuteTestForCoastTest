/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "EscapeRenderer.h"
#include "Dbg.h"

//---- EscapeRenderer ---------------------------------------------------------------
RegisterRenderer(EscapeRenderer);

EscapeRenderer::EscapeRenderer(const char *name) : Renderer(name) { }

EscapeRenderer::~EscapeRenderer() { }

void EscapeRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(EscapeRenderer.RenderAll);

	String toEscape = config["ToEscape"].AsString();
	String escapeChar = config["EscapeChar"].AsString();
	if ( toEscape != "" ) {
		char eChar = '\\';
		if (escapeChar != "") {
			eChar = escapeChar[0L];
		}
		String original;
		RenderOnString(original, c, config["String"]);
		Trace("original String [" << original << "]");
		long sz = original.Length();
		for (long i = 0; i < sz; ++i) {
			if (toEscape.StrChr(original[i]) >= 0) {
				Trace("escaping char [" << original[i] << "] at pos:" << i);
				reply << eChar;
			}
			reply << original[i];
		}

	} else {
		Render(reply, c, config["String"]);
	}
}
