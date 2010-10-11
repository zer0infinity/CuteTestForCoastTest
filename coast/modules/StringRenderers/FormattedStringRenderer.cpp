/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface ------------
#include "FormattedStringRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- FormattedStringRenderer ---------------------------------------------------------------
RegisterRenderer(FormattedStringRenderer);

FormattedStringRenderer::FormattedStringRenderer(const char *name) : Renderer(name) { }

FormattedStringRenderer::~FormattedStringRenderer() { }

void FormattedStringRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(FormattedStringRenderer.RenderAll);
	TraceAny(config, "config");

	String align, value, filler;
	long width, nSpaces = 4;
	ROAnything roaSlotConfig;
	if (config.LookupPath(roaSlotConfig, "Value")) {
		RenderOnString(value, c, roaSlotConfig);
	} else {
		reply << "Error in FormattedStringRenderer::RenderAll, Value not defined";
		return;
	}
	Trace("Value: [" << value << "]");
	if (config.LookupPath(roaSlotConfig, "Align")) {
		RenderOnString(align, c, roaSlotConfig);
	} else {
		align = "left";
	}
	Trace("Align: [" << align << "]");
	if (config.LookupPath(roaSlotConfig, "Filler")) {
		RenderOnString(filler, c, roaSlotConfig);
	} else {
		filler = " ";
	}
	Trace("Filler: [" << filler << "]");
	if (config.LookupPath(roaSlotConfig, "Width")) {
		String sWidth;
		RenderOnString(sWidth, c, roaSlotConfig);
		width = sWidth.AsLong(value.Length());
	} else {
		width = value.Length();
	}
	Trace("Width: " << width);
	if (config.LookupPath(roaSlotConfig, "SpacesForTab")) {
		String sWidth;
		RenderOnString(sWidth, c, roaSlotConfig);
		nSpaces = sWidth.AsLong(nSpaces);
	}
	Trace("SpacesForTab: " << nSpaces);

	String result(128);
	long lLeftFill = 0, lRightFill = 0;
	// string is longer than field width, trim string
	if ( value.Length() > width ) {
		value.Trim( width );
	} else {
		if ( align.IsEqual("right") ) {
			lLeftFill = width - value.Length();
		} else if ( align.IsEqual("center") ) {
			lLeftFill = ( width - value.Length() ) / 2L;
			lRightFill = width - value.Length() - lLeftFill;
		} else {
			// default: left alignment
			lRightFill = width - value.Length();
		}
	}
	Trace("LeftFills: " << lLeftFill << ", RightFill : " << lRightFill );
	for ( long iL = 0; iL < lLeftFill; ++iL) {
		result << filler;
	}
	{
		// fill spaces within string with given filler
		for (long i = 0, szv = value.Length(); i < szv; ++i) {
			if (value[i] == ' ') {
				result << filler;
			} else if (value[i] == '\t') {
				for (long l = 0; l < nSpaces; ++l) {
					result << filler;
				}
			} else {
				result << value[i];
			}
			//Trace("Result: [" << result << "] value: [" << value << "] posn:" << posn);
		}
	}
	for ( long iR = 0; iR < lRightFill; ++iR ) {
		result << filler;
	}
	Trace("Rendered Value: [" << result << "]");
	reply << result;
}
