/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPHeaderRenderer.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- HTTPHeaderRenderer ---------------------------------------------------------------
RegisterRenderer(HTTPHeaderRenderer);

HTTPHeaderRenderer::HTTPHeaderRenderer(const char *name) : Renderer(name) { }

HTTPHeaderRenderer::~HTTPHeaderRenderer() { }

void HTTPHeaderRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(HTTPHeaderRenderer.RenderAll);
	String slotname;
	if (config.IsDefined("HeaderSlot")) {
		slotname = Renderer::RenderToString(ctx, config["HeaderSlot"]);
	} else {
		slotname = Renderer::RenderToString(ctx, config);
	}
	Trace("using slotname: " << slotname);
	RenderHeader(reply, ctx, ctx.Lookup(slotname));
}

void HTTPHeaderRenderer::RenderHeader(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(HTTPHeaderRenderer.RenderHeader);
	for (long i = 0; i < config.GetSize(); i++) {
		String slot = config.SlotName(i);
		if (slot.Length() == 0) {
			Renderer::Render(reply, ctx, config[i]);
		} else {
			String slotUpperCase(slot);
			slotUpperCase.ToUpper();
			if ( slotUpperCase == "SET-COOKIE" ) {
				RenderMultipleLineHeaderField(reply, ctx, slot, config[i]);
			} else {
				reply << slot << ": ";
				RenderValues(reply, ctx, config[i]);
				reply << ENDL;
			}
		}
	}
}

void HTTPHeaderRenderer::RenderValues(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(HTTPHeaderRenderer.RenderValues);
	Render(reply, ctx, config[0L]);
	for (long i = 1; i < config.GetSize(); i++) {
		reply << ", ";
		Renderer::Render(reply, ctx, config[i]);
	}
}

void HTTPHeaderRenderer::RenderMultipleLineHeaderField(ostream &reply, Context &ctx, const String &slot, const ROAnything &config)
{
	StartTrace(HTTPHeaderRenderer.RenderMultipleLineHeaderField);
	for (long i = 0; i < config.GetSize(); i++) {
		reply << slot << ": ";
		Renderer::Render(reply, ctx, config[i]);
		reply << ENDL;
	}
}

//SOP's spike for MIMEHeader Symmetry
//void HTTPHeaderRenderer::RenderValues(ostream &reply, Context &ctx, const ROAnything &config)
//{
//	//?? use quotes? and comma , ?
//	if (config.SlotName(0L))
//		reply << config.SlotName(0L) << '=';
//	Render(reply,ctx,config[0L]);
//	for(long i=1; i<config.GetSize(); i++)
//	{
//		if (config.SlotName(i))
//			reply << "; "<<config.SlotName(i)<<'=';
//		else
//			reply << ", ";
//		Renderer::Render(reply,ctx,config[i]);
//	}
//}

