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
#include "Dbg.h"
#include "Timers.h"

//---- SlotnameOutputMapper ------------------------------------------------------------------
RegisterResultMapper(SlotnameOutputMapper);

bool SlotnameOutputMapper::DoPutAny(const char *key, Anything value, Context &ctx, ROAnything config)
{
	StartTrace1(SlotnameOutputMapper.DoPutAny, "Key: " << NotNull(key));

	Anything dest = GetDestination(ctx, config);
	Anything rows = value;
	ROAnything slotnameSlots = config["SlotnameSlots"];
	long depth = slotnameSlots.GetSize();
	long sz = rows.GetSize();
	DAAccessTimer(SlotnameOutputMapper.DoPutAny, sz << " entries procesed", ctx);
	for (long i = 0; i < sz; i++) {
		Anything temp = dest;
		String slotname;
		for (long j = 0; j < depth; j++) {
			String slotnameSlot = slotnameSlots[j].AsString();
			slotname = rows[i][slotnameSlot].AsString();
			if (j < depth - 1) {
				if (!temp.IsDefined(slotname)) {
					temp[slotname] = MetaThing();
				}
			} else {
				temp[slotname] = rows[i];
			}
			temp = temp[slotname];
			TraceAny(dest, "Dest so far");
		}
		TraceAny(dest, "Dest after row");
	}
	TraceAny(dest, "Dest finally");
	return true;
}
