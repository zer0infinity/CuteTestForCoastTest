/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TimeStampRenderer.h"

//--- project modules used -----------------------------------------------------
#include "TimeStamp.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- TimeStampRenderer ---------------------------------------------------------------
RegisterRenderer(TimeStampRenderer);

TimeStampRenderer::TimeStampRenderer(const char *name) : Renderer(name) { }

TimeStampRenderer::~TimeStampRenderer() { }

void TimeStampRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(TimeStampRenderer.RenderAll);
	ROAnything roaUxTimeSpec;
	TimeStamp aStamp;
	if ( !config.LookupPath(roaUxTimeSpec, "UnixTime") && config.GetSize() ) {
		roaUxTimeSpec = config[0L];
	}
	TraceAny(roaUxTimeSpec, "UnixTime-spec");
	if ( !roaUxTimeSpec.IsNull() ) {
		time_t tTime = Renderer::RenderToStringWithDefault(ctx, roaUxTimeSpec, 0L).AsLong(0L);
		aStamp = TimeStamp(tTime);
	}
	reply << aStamp.AsString();
}
