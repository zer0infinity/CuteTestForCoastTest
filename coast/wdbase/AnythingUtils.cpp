/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnythingUtils.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Renderer.h"
#include "Dbg.h"

//-- StoreCopier ---------------------------------------------------------------
void StoreCopier::Operate(Context &c, Anything &dest, const Anything &config, char delim, char indexdelim)
{
	StartTrace(StoreCopier.Operate);
	ROAnything ROconfig(config);
	Operate(c, dest, ROconfig, delim, indexdelim);
}

void StoreCopier::Operate(Context &c, Anything &dest, const ROAnything &config, char delim, char indexdelim)
{
	StartTrace(StoreCopier.Operate);
	TraceAny(config, "Config");

	Trace("delim [" << delim << "], indexdelim [" << indexdelim << "]");
	long sz = config.GetSize();
	for (long i = 0; i < sz; i++) {
		String sourceLookupName = config.SlotName(i);
		String destSlot;
		Renderer::RenderOnString(destSlot, c, config[i]);
		if ( sourceLookupName && destSlot ) {
			dest[destSlot] = c.Lookup(sourceLookupName, delim, indexdelim).DeepClone();
		}
	}
}

//-- StoreFinder ---------------------------------------------------------------

void StoreFinder::Operate(Context &c, Anything &dest, const Anything &config)
{
	StartTrace(StoreFinder.Operate);
	ROAnything ROconfig(config);
	Operate(c, dest, ROconfig);
}

void StoreFinder::Operate(Context &context, Anything &dest, const ROAnything &config)
{
	StartTrace(StoreFinder.Operate);
	TraceAny(config, "Config");

	String store = config["Store"].AsString("");
	dest = FindStore(context, store);

	String destSlotname;
	Renderer::RenderOnString(destSlotname, context, config["Slot"]);
	Trace("Destination slotname [" << destSlotname << "]");
	Anything anyConfig;
	anyConfig["Slot"] = destSlotname;
	if (config.IsDefined("Delim")) {
		anyConfig["Delim"] = config["Delim"].AsCharPtr(".")[0L];
	}
	if (config.IsDefined("IndexDelim")) {
		anyConfig["IndexDelim"] = config["IndexDelim"].AsCharPtr(":")[0L];
	}

	SlotFinder::Operate(dest, dest, anyConfig);
}

Anything &StoreFinder::FindStore(Context &c, String &storeName)
{
	StartTrace(StoreFinder.FindStore);

	if ( storeName == "Role" ) {
		return c.GetRoleStoreGlobal();
	}
	if ( storeName == "Session" ) {
		return c.GetSessionStore();
	}
	if ( storeName == "Request" ) {
		return c.GetRequest();
	}
	return c.GetTmpStore();
}

//-- StorePutter ---------------------------------------------------------------

void StorePutter::Operate(Anything &source, Context &c, const Anything &config)
{
	StartTrace(StorePutter.Operate);
	ROAnything ROconfig(config);
	Operate(source, c, ROconfig);
}

void StorePutter::Operate(Anything &source, Context &c, const ROAnything &config)
{
	StartTrace(StorePutter.Operate);
	TraceAny(config, "Config");

	Anything clonedConfig = config.DeepClone();
	String destSlotname;
	Renderer::RenderOnString(destSlotname, c, config["Slot"]);
	clonedConfig["Slot"] = destSlotname;
	if (config.IsDefined("Delim")) {
		clonedConfig["Delim"] = config["Delim"].AsCharPtr(".")[0L];
	}
	if (config.IsDefined("IndexDelim")) {
		clonedConfig["IndexDelim"] = config["IndexDelim"].AsCharPtr(":")[0L];
	}

	String store = config["Store"].AsString("");
	SlotPutter::Operate(source, StoreFinder::FindStore(c, store), clonedConfig);
}
