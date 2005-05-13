/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "GetEnvRenderer.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- GetEnvRenderer ---------------------------------------------------------------
RegisterRenderer(GetEnvRenderer);

GetEnvRenderer::GetEnvRenderer(const char *name)
	: LookupRenderer(name)
	, fTheEnvironment(Storage::Global())
{
	StartTrace(GetEnvRenderer.GetEnvRenderer);
	System::GetProcessEnvironment(fTheEnvironment);
	TraceAny(fTheEnvironment, "the environment");
}

GetEnvRenderer::~GetEnvRenderer() { }

ROAnything GetEnvRenderer::DoLookup(Context &context, const char *name, char delim, char indexdelim)
{
	StartTrace1(GetEnvRenderer.DoLookup, "LookupName [" << NotNull(name) << "]");
	ROAnything roaResult;
	((ROAnything)fTheEnvironment).LookupPath(roaResult, name, delim, indexdelim);
	TraceAny(roaResult, "result");
	return roaResult;
}
