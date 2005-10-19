/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface ------------
#include "ReplaceExtChrRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- ReplaceExtChrRenderer ---------------------------------------------------------------
RegisterRenderer(ReplaceExtChrRenderer);

ReplaceExtChrRenderer::ReplaceExtChrRenderer(const char *name) : Renderer(name) { }

ReplaceExtChrRenderer::~ReplaceExtChrRenderer() { }

void ReplaceExtChrRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(ReplaceExtChrRenderer.RenderAll);

	TraceAny(config, "config");

	String strDestination, strTmpSource, strSource;
	RenderOnString(strSource, c, config["StringToReplace"]);
	strDestination = strTmpSource = strSource;
	ROAnything roaMapAny;
	if (config.LookupPath(roaMapAny, "ReplaceStrings")) {
		TraceAny(roaMapAny, " config");
		long lsize = roaMapAny.GetSize();
		if ( lsize > 0 ) {
			strDestination = "";
			for (long cfgIdx = 0L; cfgIdx < lsize; cfgIdx++) { // loop over replacestrings-list
				String currSlot = roaMapAny.SlotName(cfgIdx);
				String currValue = roaMapAny[currSlot].AsString("");
				//TODO: prevent loop for each replacestring -> RegExpRenderer
				String rString = currSlot;
				long startIdx = 0L, lstIdx = 0L;
				long lsize = strTmpSource.Length();
				// returns -1 if there no rString can be found
				while ((startIdx = strTmpSource.Contains(rString)) >= 0L) {
					String tmpString = strTmpSource.SubString(lstIdx, startIdx); //
					strDestination.Append(tmpString);
					strDestination.Append(currValue);
					lstIdx = (startIdx + rString.Length());
					strTmpSource = strTmpSource.SubString(lstIdx, lsize);
					lsize = strTmpSource.Length();
				}
				if (lstIdx > 0L) {
					//
					strDestination.Append(strTmpSource);
					strTmpSource = "";
					strTmpSource = strDestination;
				}
			}
			if (strDestination.Length() <= 0L) {
				strDestination = strSource;
			}
		}
	}
	reply << strDestination;
}
