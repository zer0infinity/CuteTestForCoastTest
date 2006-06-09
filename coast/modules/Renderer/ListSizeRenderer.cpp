/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ListSizeRenderer.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- ListSizeRenderer ---------------------------------------------------------------
RegisterRenderer(ListSizeRenderer);

ListSizeRenderer::ListSizeRenderer(const char *name) : Renderer(name) { }

void ListSizeRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(ListSizeRenderer.RenderAll);

	ROAnything data;
	if (!config.LookupPath(data, "ListData", '\000')) {
		data = config[0L];
	}

	if ( !data.IsNull() ) {
		String str;
		RenderOnString(str, ctx, data);
		ROAnything roaData = ctx.Lookup(str);
		if ( !roaData.IsNull() ) {
			long size = roaData.GetSize();
			Trace("ListData size is: " << size);
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
