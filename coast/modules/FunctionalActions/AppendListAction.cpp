/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface ------------
#include "AppendListAction.h"

//--- standard modules used ----------------------------------------------------
#include "AnythingUtils.h"
#include "Renderer.h"
#include "Dbg.h"

class ContextSlotAppender
{
public:
	ContextSlotAppender() {};
	virtual ~ContextSlotAppender() {};

	virtual void Operate(Context &c, Anything &dest, const Anything &config);
	virtual void Operate(Context &c, Anything &dest, const ROAnything &config);
};

void ContextSlotAppender::Operate(Context &c, Anything &dest, const Anything &config)
{
	StartTrace(ContextSlotAppender.Operate);
	ROAnything ROconfig(config);
	Operate(c, dest, ROconfig);
}

void ContextSlotAppender::Operate(Context &c, Anything &dest, const ROAnything &config)
{
	StartTrace(ContextSlotAppender.Operate);
	TraceAny(config, "Config");

	long sz = config.GetSize(), idx;
	for (long i = 0; i < sz; i++) {
		String sourceLookupName = config.SlotName(i);
		String destSlot;
		Renderer::RenderOnString(destSlot, c, config[i]);
		Trace("slot to lookup [" << sourceLookupName << "] destslot [" << destSlot << "]");
		ROAnything roaLookedUp;
		if ( sourceLookupName && destSlot && c.Lookup(sourceLookupName, roaLookedUp)) {
			TraceAny(((ROAnything)dest)[destSlot], "current destination");
			Anything res;
			for ( idx = 0; idx < dest[destSlot].GetSize(); idx++ ) {
				res[ String() << idx ] = dest[destSlot][idx];
			}
			Anything src = roaLookedUp.DeepClone();
			TraceAny(src, "anything to append");
			for ( long newIdx = 0; newIdx < src.GetSize(); newIdx++ ) {
				res[ String() << idx + newIdx ] = src[newIdx];
			}
			dest[destSlot] = res;
			TraceAny(res, "new destination");
		}
	}
}

//---- CopyContextAction ----------------------------------------------------------------
RegisterAction(AppendListAction);

AppendListAction::AppendListAction(const char *name) : CopyAction(name) { }

AppendListAction::~AppendListAction() { }

void AppendListAction::Copy(Anything &dest, const ROAnything &copyList, const ROAnything &config, Context &ctx)
{
	StartTrace(CopyContextAction.Copy);

	ContextSlotAppender appender;
	appender.Operate(ctx, dest, copyList);
}

