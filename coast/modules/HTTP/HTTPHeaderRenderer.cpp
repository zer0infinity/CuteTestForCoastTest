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
	namespace constants {
		const char * const headerSlot = "HeaderSlot";
	}

	void RenderHeader(std::ostream &reply, Context &ctx, const ROAnything &config) {
		StartTrace(HTTPHeaderRenderer.RenderHeader);
		TraceAny(config, "HTTPHeaders to render to stream");
		AnyExtensions::Iterator<ROAnything> headerStructureIter(config);
		ROAnything fieldValues;
		String strSlotname;
		while (headerStructureIter.Next(fieldValues)) {
			if ( not headerStructureIter.SlotName(strSlotname) ) {
				//! prepared "header: value" entry or a Renderer specification
				Renderer::Render(reply, ctx, fieldValues);
				reply << coast::http::constants::newLine;
			} else {
				coast::http::putHeaderFieldToStream(reply, ctx, strSlotname, fieldValues);
			}
		}
	}
}

void HTTPHeaderRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(HTTPHeaderRenderer.RenderAll);
	String slotname;
	if (config.IsDefined(constants::headerSlot)) {
		slotname = Renderer::RenderToString(ctx, config[constants::headerSlot]);
	} else {
		slotname = Renderer::RenderToString(ctx, config);
	}
	Trace("using slotname: " << slotname);
	RenderHeader(reply, ctx, ctx.Lookup(slotname));
}
