/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "GetEnvRenderer.h"
#include "SystemBase.h"
#include "Tracer.h"

//---- GetEnvRenderer ---------------------------------------------------------------
RegisterRenderer(GetEnvRenderer);

GetEnvRenderer::GetEnvRenderer(const char *name)
	: LookupRenderer(name)
{
}

GetEnvRenderer::~GetEnvRenderer() { }

ROAnything GetEnvRenderer::DoLookup(Context &context, const char *name, char delim, char indexdelim)
{
	StartTrace1(GetEnvRenderer.DoLookup, "LookupName [" << NotNull(name) << "]");
	Anything anyEnv;
	Coast::System::GetProcessEnvironment(anyEnv);
	context.GetTmpStore()["_GetEnvRenderer_"] = anyEnv;
	ROAnything roaResult;
	((ROAnything)anyEnv).LookupPath(roaResult, name, delim, indexdelim);
	TraceAny(roaResult, "result");
	return roaResult;
}
