/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ListSizeRenderer.h"
#include "Dbg.h"

//---- ListSizeRenderer ---------------------------------------------------------------
RegisterRenderer(ListSizeRenderer);

ListSizeRenderer::ListSizeRenderer(const char *name) : Renderer(name) { }

void ListSizeRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(ListSizeRenderer.RenderAll);

	ROAnything data;
	if (!config.LookupPath(data, "ListName", '\000')) {
		data = config[0L];
	}

	if ( !data.IsNull() ) {
		String str;
		RenderOnString(str, ctx, data);
		Trace("ListName is [" << str << "]");
		ROAnything roaData = ctx.Lookup(str);
		if ( !roaData.IsNull() ) {
			long size = roaData.GetSize();
			Trace("size of [" << str << "] is: " << size);
			reply << size;
			return;
		}
	}
	ROAnything dft;
	if (!config.LookupPath(dft, "Default", '\000')) {
		dft = config[1L];
	}
	if (dft.GetType() != AnyNullType) {
		Render(reply, ctx, dft);
	}
}
