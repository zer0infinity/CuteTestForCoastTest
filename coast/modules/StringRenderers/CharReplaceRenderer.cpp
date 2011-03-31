/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "CharReplaceRenderer.h"
#include "Dbg.h"

//---- CharReplaceRenderer ---------------------------------------------------------------
RegisterRenderer(CharReplaceRenderer);

CharReplaceRenderer::CharReplaceRenderer(const char *name) : Renderer(name) { }

CharReplaceRenderer::~CharReplaceRenderer() { }

void CharReplaceRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(CharReplaceRenderer.RenderAll);

	String charsToReplace = config["CharsToReplace"].AsString();
	String replaceChar = config["ReplaceChar"].AsString("_");
	if ( charsToReplace != "" ) {
		String original;
		RenderOnString(original, c, config["String"]);
		if (replaceChar != "") {
			// replace characters
			char rChar = replaceChar[0L];
			long sz = charsToReplace.Length();
			long charpos = 0L;
			for (long i = 0; i < sz; ++i) {
				while ((charpos = original.StrChr(charsToReplace[i], charpos)) >= 0L) {
					original.PutAt(charpos, rChar);
				}
			}
			reply << original;
		} else {
			// remove characters
			long sz = original.Length();
			for (long i = 0; i < sz; ++i) {
				if (charsToReplace.StrChr(original[i]) < 0) {
					reply << original[i];
				}
			}
		}
	} else {
		Render(reply, c, config["String"]);
	}
}
