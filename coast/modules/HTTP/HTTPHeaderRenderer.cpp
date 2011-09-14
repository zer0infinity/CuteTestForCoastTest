/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "HTTPHeaderRenderer.h"
#include "AnyIterators.h"
#include "RE.h"
#include "HTTPConstants.h"

RegisterRenderer(HTTPHeaderRenderer);

namespace {
    const char *_HeaderSlot = "HeaderSlot";

	void RenderHeader(std::ostream &reply, Context &ctx, const ROAnything &config) {
		StartTrace(HTTPHeaderRenderer.RenderHeader);
		//!@FIXME: use precompiled RE-Program as soon as RE's ctor takes an ROAnything as program
		AnyExtensions::Iterator<ROAnything> headerStructureIter(config);
		ROAnything fieldValues;
		String strSlotname;
		while (headerStructureIter.Next(fieldValues)) {
			if ( not headerStructureIter.SlotName(strSlotname) ) {
				//! prepared "header: value" entry or a Renderer specification
				Renderer::Render(reply, ctx, fieldValues);
				reply << Coast::HTTP::_newLine;
			} else {
				Coast::HTTP::putHeaderFieldToStream(reply, ctx, strSlotname, fieldValues);
			}
		}
	}
}

void HTTPHeaderRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(HTTPHeaderRenderer.RenderAll);
	String slotname;
	if (config.IsDefined(_HeaderSlot)) {
		slotname = Renderer::RenderToString(ctx, config[_HeaderSlot]);
	} else {
		slotname = Renderer::RenderToString(ctx, config);
	}
	Trace("using slotname: " << slotname);
	RenderHeader(reply, ctx, ctx.Lookup(slotname));
}
