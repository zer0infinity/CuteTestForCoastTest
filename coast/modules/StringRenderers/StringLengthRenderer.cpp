/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "StringLengthRenderer.h"
RegisterRenderer(StringLengthRenderer);

void StringLengthRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(StringLengthRenderer.RenderAll);
	ROAnything roaSlotConfig;
	String strValue;
	if (config.LookupPath(roaSlotConfig, "Value")) {
		RenderOnString(strValue, ctx, roaSlotConfig);
	} else {
		RenderOnString(strValue, ctx, config);
	}
	Trace("Value: [" << strValue << "]");
	reply << strValue.Length();
}
