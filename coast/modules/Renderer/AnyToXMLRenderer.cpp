/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnyToXMLRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- AnyToXMLRenderer ---------------------------------------------------------
RegisterRenderer(AnyToXMLRenderer);

AnyToXMLRenderer::AnyToXMLRenderer(const char *name) : Renderer(name)
{
}

AnyToXMLRenderer::~AnyToXMLRenderer()
{
}

void AnyToXMLRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(AnyToXMLRenderer.RenderAll);
	TraceAny(config, "config");

	// Check mandatory slots
	ROAnything inputInfo;
	if (! config.LookupPath(inputInfo, "Input")) {
		return;
	}

	// Get the Input Anything

	String inputAnyName;
	RenderOnString(inputAnyName, c, inputInfo);

	ROAnything input = c.Lookup(inputAnyName);
	if ( !input.IsNull() ) {
		// We have found the input - lets start
		RenderXML(reply, input);
	}
}

void AnyToXMLRenderer::RenderXML(ostream &reply, ROAnything &input)
{
	StartTrace(AnyToXMLRenderer.RenderXML);
	TraceAny(input, "Input");

	// Check pre condition
	if	( input.GetSize() < 1 || !input.SlotName(0)
		  || HasUnnamedChilds(ROAnything(input[0L])) ) {
		Trace("Input not wellformed");
		return;
	}

	RenderNamedChilds(reply, input);
}

void AnyToXMLRenderer::RenderNamedChilds(ostream &reply, ROAnything &list)
{
	StartTrace(AnyToXMLRenderer.RenderNamedChilds);

	long sz = list.GetSize();
	for ( long i = 0; i < sz; i++) {
		String slotname = list.SlotName(i);
		if (!slotname.Length()) {
			SysLog::Error("Unnamed child in RenderNamedChilds found");
			continue;
		}

		ROAnything element = list[i];
		if (HasUnnamedChilds(element)) {
			// Unnamed childs -> list of slotname items
			RenderUnnamedChilds(reply, slotname, element);
		} else {
			reply << "<" << slotname << ">";
			if (element.GetType() == Anything::eArray) {
				// Childs with slotnames
				RenderNamedChilds(reply, element);
			} else {
				reply << element.AsString("");
			}
			reply << "</" << slotname << ">";
		}
	}
}

void AnyToXMLRenderer::RenderUnnamedChilds(ostream &reply, String &tagname, ROAnything &list)
{
	StartTrace(AnyToXMLRenderer.RenderUnnamedChilds);

	long sz = list.GetSize();
	for ( long i = 0; i < sz; i++) {

		ROAnything element = list[i];
		if (HasUnnamedChilds(element)) {
			// Unnamed childs -> should not happen
			SysLog::Error("Unnamed childs of an unnamed child found");
		} else {
			reply << "<" << tagname << ">";
			if (element.GetType() == Anything::eArray) {
				// Childs with slotnames
				RenderNamedChilds(reply, element);
			} else {
				reply << element.AsString("");
			}
			reply << "</" << tagname << ">";
		}
	}
}

bool AnyToXMLRenderer::HasUnnamedChilds(ROAnything element)
{
	return (element.GetType() == Anything::eArray && !element.SlotName(0));
}

