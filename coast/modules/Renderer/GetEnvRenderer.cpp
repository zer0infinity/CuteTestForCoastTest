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
	System::GetProcessEnvironment(fTheEnvironment);
}

GetEnvRenderer::~GetEnvRenderer() { }
ROAnything GetEnvRenderer::DoLookup(Context &context, const char *name, char delim, char indexdelim)
{
	Anything result(fTheEnvironment.GetAllocator()); // avoid cloning
	fTheEnvironment.LookupPath(result, name, delim, indexdelim);
	return result;
}

