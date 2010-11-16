/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnythingLookupPathResultMapper.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- AnythingLookupPathResultMapper ----------------------------------------------------------------
RegisterResultMapper(AnythingLookupPathResultMapper);

bool AnythingLookupPathResultMapper::DoPutAnyWithSlotname(const char *key, Anything &value, Context &ctx, ROAnything roaScript, const char *slotname)
{
	StartTrace1(AnythingLookupPathResultMapper.DoPutAnyWithSlotname, "key [" << NotNull(key) << "] slotname [" << NotNull(slotname) << "]");
	Trace("Using slotname [" << slotname << "] as Lookup path in value");
	bool bRet = true;	// do not fail when lookup fails!
	Anything anyValue;
	if ( value.LookupPath(anyValue, slotname) ) {
		TraceAny(anyValue, "Calling myself again with Anything looked up at [" << slotname << "]");
		bRet = DoPutAny(key, anyValue, ctx, roaScript);
		Trace("RetCode of DoPutAny:" << (bRet ? "true" : "false"));
	}
	return bRet;
}
