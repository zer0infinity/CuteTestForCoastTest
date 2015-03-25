/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "Context.h"
#include "Page.h"
#include "PageRelatedLookupRenderer.h"

//---- PageRelatedLookupRenderer ----------------------------------------------------------------
RegisterRenderer(PageRelatedLookupRenderer);

PageRelatedLookupRenderer::PageRelatedLookupRenderer(const char *name) : LookupRenderer(name)
{}

ROAnything PageRelatedLookupRenderer::DoLookup(Context &context, const char *name, char delim, char indexdelim)
{
	ROAnything a;
	Page *page = context.GetPage();
	if (page) {
		// lookup in page specific array
		ROAnything config;
		String pName;
		page->GetName(pName);
		config = context.Lookup(pName, delim, indexdelim);

		// lookup in page specific array
		if (!config.LookupPath(a, name, delim, indexdelim)) {
			// lookup in whole context
			a = context.Lookup(name, delim, indexdelim);
		}
	}
	return a;
}
