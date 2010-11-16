/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RegExpFilterFieldsResultMapper.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "RE.h"
#include "AnyIterators.h"

//---- RegExpFilterFieldsResultMapper ----------------------------------------------------------------
RegisterResultMapper(RegExpFilterFieldsResultMapper);

bool RegExpFilterFieldsResultMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RegExpFilterFieldsResultMapper.DoPutAny, "key [" << NotNull(key) << "]");
	if ( script.IsNull() || not script.GetSize() ) {
		return true;
	}
	return ResultMapper::DoPutAny(key, value, ctx, script);
}

bool RegExpFilterFieldsResultMapper::DoPutAnyWithSlotname(const char *key, Anything &value, Context &ctx, ROAnything roaScript, const char *slotname)
{
	StartTrace1(RegExpFilterFieldsResultMapper.DoPutAnyWithSlotname, "key [" << NotNull(key) << "] regular expression [" << NotNull(slotname) << "]");
	TraceAny(value, "value");
	RE aRE(slotname, static_cast<RE::eMatchFlags>(Lookup("MatchFlags", 0L)));

	AnyExtensions::Iterator<Anything> slotnameIterator(value);
	Anything anyValue, anyProcessed;
	String strSlotname;
	bool bMappingSuccess = true, bMappedValues=false;
	while ( slotnameIterator.Next(anyValue) && bMappingSuccess ) {
		if ( slotnameIterator.SlotName(strSlotname) && aRE.ContainedIn(strSlotname) ) {
			anyProcessed.Append(slotnameIterator.Index());
			TraceAny(anyValue, "pattern [" << slotname << "] matched in slotname [" << strSlotname << "] with value:");
			if ( roaScript.AsString().IsEqual("-") ) {
				continue; //<! shortcut because we got a negative filter which is not to put through
			}
			bMappedValues = true;
			String strNewKey = key;
			strNewKey.Append(getDelim()).Append(strSlotname);
			Trace("Calling ResultMapper::DoPut() with new key [" << strNewKey << "]");
			if ( roaScript.IsNull() || not roaScript.GetSize() ) {
				//! catch emtpy mapper script and directly use final put
				bMappingSuccess = DoFinalPutAny(strNewKey, anyValue, ctx) && bMappingSuccess;
			} else {
				bMappingSuccess = DoPutAny(strNewKey, anyValue, ctx, roaScript) && bMappingSuccess;
			}
			Trace("RetCode of DoPutAny:" << (bMappingSuccess ? "true" : "false"));
		}
	}
	for ( long idx = anyProcessed.GetSize()-1l; idx >= 0L; --idx ) {
		Trace("removing processed entry at index " << anyProcessed[idx].AsLong(-1L));
		value.Remove(anyProcessed[idx].AsLong(-1L));
	}
	return (bMappedValues ? bMappingSuccess : Lookup("NoMatchReturnValue", 1L) > 0L );
}
