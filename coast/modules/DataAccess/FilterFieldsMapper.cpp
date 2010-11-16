/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "FilterFieldsMapper.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- FilterFieldsMapper ------------------------------------------------------------------
RegisterResultMapper(FilterFieldsMapper);

bool FilterFieldsMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script)
{
	StartTrace1(FilterFieldsMapper.DoPutAny, NotNull(key));
	TraceAny(value, "value to put");
	ROAnything roaFieldList;
	Anything anyNew, anyFilterValue;
	if ( Lookup("FieldList", roaFieldList) ) {
		const char *pSlotname( NULL );
		for ( long lIdx = 0, lSize = roaFieldList.GetSize(); lIdx < lSize; ++lIdx) {
			pSlotname = roaFieldList[lIdx].AsCharPtr();
			if ( pSlotname != NULL && value.LookupPath( anyFilterValue, pSlotname ) ) {
				if ( pSlotname[0L] == getIndexDelim() ) {
					Trace("appending value in output because it was an indexed lookup [" << pSlotname << "]");
					anyNew.Append( anyFilterValue );
				} else {
					anyNew[pSlotname] = anyFilterValue;
				}
			}
		}
	} else {
		// using everything
		anyNew = value;
	}
	TraceAny(anyNew, "final value to put");
	return ResultMapper::DoPutAny(key, anyNew, ctx, script);
}
