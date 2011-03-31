/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RemoteStresser.h"
#include "DataAccess.h"
#include "DiffTimer.h"
#include "Context.h"
#include "Dbg.h"

//---- RemoteStresser -----------------------------------------------------------
RegisterStresser(RemoteStresser);

RemoteStresser::RemoteStresser(const char *RemoteStresserName)
	: Stresser(RemoteStresserName)
{
	StartTrace(RemoteStresser.Ctor);
}

RemoteStresser::~RemoteStresser()
{
	StartTrace(RemoteStresser.Destructor);
}

Anything RemoteStresser::Run(long id)
{
	StartTrace(RemoteStresser.Run);
	TraceAny(fConfig, "Config");

	Anything results;
	// get the DataAccess's name from configuration
	String daName = fConfig["DataAccess"].AsString("");
	if (daName != "") {
		// Create an empty Context and pass the stressers config into its environment store
		Context ctx(fConfig.DeepClone(), Anything(), 0, 0, 0);
		DataAccess da(daName);
		DiffTimer timer;
		if ( da.StdExec(ctx) ) {
			Anything tmpStore = ctx.GetTmpStore();
			TraceAny(tmpStore, "TmpStore");
			results = tmpStore["Mapper"]["Output"];
		}
		Trace("AccessTime " << timer.Diff());
	}
	if (results.IsNull()) {
		results[0L]["Nr"] = 1L;
		results[0L]["Steps"] = 1L;
		results[0L]["Error"] = 1L;
		results[0L]["ErrorMessageCtr"]["1"][0L] = "DataAccess failed or result slot was empty (Mapper.Output)";
	}
	return results;
}
