/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface ------------
#include "FormattedDateRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- FormattedDateRenderer ---------------------------------------------------------------
RegisterRenderer(FormattedDateRenderer);

FormattedDateRenderer::FormattedDateRenderer(const char *name) : Renderer(name) { }

FormattedDateRenderer::~FormattedDateRenderer() { }

void FormattedDateRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(FormattedDateRenderer.RenderAll);
	TraceAny(config, "config");

	String value;
	ROAnything roaSlotConfig;
	if (config.LookupPath(roaSlotConfig, "Value")) {
		RenderOnString(value, c, roaSlotConfig);
	} else {
		reply << "Error in FormattedDateRenderer::RenderAll, Value not defined";
		return;
	}
	Trace("Value: [" << value << "]");

	// Conversion from "MMM TT JJJJ hh:mmAM" into "TT MMM JJJJ"
	//                 "0123456789012345678"

	String result;
	if ( value.Length() == 19 ) {
		result << value.At(4) << value.At(5) << " "
			   << value.At(0) << value.At(1) << value.At(2) << " "
			   << value.At(7) << value.At(8) << value.At(9) << value.At(10);
	}
	Trace("Rendered Value: [" << result << "]");
	reply << result;
}
