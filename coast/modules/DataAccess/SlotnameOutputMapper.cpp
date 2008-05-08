/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SlotnameOutputMapper.h"

//--- standard modules used ----------------------------------------------------
#include "AnyIterators.h"
#include "Dbg.h"
#include "Timers.h"

//---- SlotnameOutputMapper ------------------------------------------------------------------
RegisterResultMapper(SlotnameOutputMapper);

bool SlotnameOutputMapper::DoPutAny(const char *key, Anything value, Context &ctx, ROAnything config)
{
	StartTrace1(SlotnameOutputMapper.DoPutAny, "Key: " << NotNull(key));

	DAAccessTimer(SlotnameOutputMapper.DoPutAny, value.GetSize() << " entries processed", ctx);
	Anything dest = GetDestination(ctx, config);
	AnyExtensions::Iterator<Anything> aRowIter(value);
	Anything anyRow( value.GetAllocator() );
	String strDestPath(128L);
	while ( aRowIter(anyRow) ) {
		SubTraceAny(PerRow, anyRow, "entry");
		strDestPath.Trim(0);
		AnyExtensions::Iterator<ROAnything> aSlotnameIter(config["SlotnameSlots"]);
		ROAnything roaSNEntry;
		while ( aSlotnameIter(roaSNEntry) ) {
			String strKey = ((ROAnything)anyRow)[roaSNEntry.AsCharPtr()].AsString();
			if ( strKey.Length() ) {
				if ( strDestPath.Length() ) {
					strDestPath << '.';
				}
				strDestPath << strKey;
			}
		}
		SubTrace(PerRow, "destination path [" << strDestPath << "]");
		SlotPutter::Operate(anyRow, dest, strDestPath, false);
		SubTraceAny(PerRow, dest, "Dest after row");
	}
	SubTraceAny(Final, dest, "Dest finally");
	return true;
}
