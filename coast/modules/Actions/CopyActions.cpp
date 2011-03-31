/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "CopyActions.h"
#include "Renderer.h"
#include "AnythingUtils.h"
#include "Dbg.h"

//---- CopyAction ----------------------------------------------------------------
// abstract base class - Not registered
CopyAction::CopyAction(const char *name) : Action(name)  { }

CopyAction::~CopyAction() { }

bool CopyAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(CopyAction.DoExecAction);

	TraceAny(config, "config");
	//check mandatory configuration slot
	ROAnything destConfig;
	ROAnything copyList;
	if (! (config.LookupPath(destConfig, "Destination") && config.LookupPath(copyList, "CopyList")) ) {
		return false;
	}

	Anything data, anydestConfig;
	anydestConfig = destConfig.DeepClone(Coast::Storage::Current());
	if (config.IsDefined("Delim")) {
		Trace("copying delim [" << config["Delim"].AsCharPtr(".")[0L] << "] to destination getter config");
		anydestConfig["Delim"] = config["Delim"].AsCharPtr(".")[0L];
	}
	if (config.IsDefined("IndexDelim")) {
		Trace("copying indexdelim [" << config["IndexDelim"].AsCharPtr(".")[0L] << "] to destination getter config");
		anydestConfig["IndexDelim"] = config["IndexDelim"].AsCharPtr(":")[0L];
	}
	GetDestination(data, anydestConfig, ctx);

	TraceAny(data, "Data before copy");
	TraceAny(copyList, "Copy list");

	Copy(data, copyList, config, ctx);

	TraceAny(data, "Data after copy");

	return true;
}

void CopyAction::GetDestination(Anything &dest, const ROAnything &destConfig, Context &ctx)
{
	StartTrace(CopyContextAction.GetDestination);
	StoreFinder::Operate(ctx, dest, destConfig);
}

//---- CopyContextAction ----------------------------------------------------------------
RegisterAction(CopyContextAction);

CopyContextAction::CopyContextAction(const char *name) : CopyAction(name) { }

CopyContextAction::~CopyContextAction() { }

void CopyContextAction::Copy(Anything &dest, const ROAnything &copyList, const ROAnything &config, Context &ctx)
{
	StartTrace(CopyContextAction.Copy);

	Trace("delim [" << config["Delim"].AsCharPtr(".")[0L] << "], indexdelim [" << config["IndexDelim"].AsCharPtr(":")[0L] << "]");
	StoreCopier::Operate(ctx, dest, copyList, config["Delim"].AsCharPtr(".")[0L], config["IndexDelim"].AsCharPtr(":")[0L]);
}

//---- CopyQueryAction ----------------------------------------------------------------
RegisterAction(CopyQueryAction);

CopyQueryAction::CopyQueryAction(const char *name) : CopyContextAction(name) { }

CopyQueryAction::~CopyQueryAction() { }

void CopyQueryAction::Copy(Anything &dest, const ROAnything &copyList, const ROAnything &config, Context &ctx)
{
	StartTrace(CopyQueryAction.Copy);

	Trace("delim [" << config["Delim"].AsCharPtr(".")[0L] << "], indexdelim [" << config["IndexDelim"].AsCharPtr(":")[0L] << "]");
	Anything &query = ctx.GetQuery();

	SlotCopier::Operate(query, dest, copyList, config["Delim"].AsCharPtr(".")[0L], config["IndexDelim"].AsCharPtr(":")[0L]);
}

//---- CopyQueryIfNotEmptyAction ----------------------------------------------------------------
RegisterAction(CopyQueryIfNotEmptyAction);

CopyQueryIfNotEmptyAction::CopyQueryIfNotEmptyAction(const char *name) : CopyQueryAction(name) { }

CopyQueryIfNotEmptyAction::~CopyQueryIfNotEmptyAction() { }

bool CopyQueryIfNotEmptyAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(CopyQueryIfNotEmptyAction.DoExecAction);

	//check mandatory configuration slot
	ROAnything copyList;
	if (! config.LookupPath(copyList, "CopyList") ) {
		return false;
	}

	Anything &query = ctx.GetQuery();
	char delim = config["Delim"].AsCharPtr(".")[0L];
	char indexdelim = config["IndexDelim"].AsCharPtr(":")[0L];

	long sz = copyList.GetSize();
	for (long i = 0; i < sz; ++i) {
		String sourceSlot = copyList.SlotName(i);
		Anything content;
		if ( sourceSlot && query.LookupPath(content, sourceSlot, delim, indexdelim) && content.AsString().Length() ) {
			Trace("Not empty slot: " << sourceSlot << " contains >" << content.AsCharPtr("") << "<");
			return CopyQueryAction::DoExecAction(transitionToken, ctx, config);
		}
	}
	return true;
}
