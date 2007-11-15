/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "PeriodicAction.h"

//--- standard modules used ----------------------------------------------------
#include "Context.h"
#include "Action.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//--- PeriodicAction ----
PeriodicAction::PeriodicAction(const String &action, long waitTime)
	: Thread((const char *)action, true)
	, fAction(action)
	, fWaitTime(waitTime)
{
}

PeriodicAction::~PeriodicAction()
{
}

void PeriodicAction::Run()
{
	StartTrace(PeriodicAction.Run);
	while ( CheckState( eRunning, 0, 1 ) ) {
		Trace("Waiting " << fWaitTime << "s for next period");
		// we will never reach eWorking, but the function will return when entering termination
		CheckRunningState(eWorking, fWaitTime);

		// execute only if we are still running and not already in termination sequence
		// never try to call CheckState() without at least a nanosecond to wait
		// -> otherwise we will block until program termination...
		if ( CheckState( eRunning, 0, 1 ) && !CheckState(eTerminationRequested, 0, 1) ) {
			Trace("Starting work and calling Action [" << fAction << "]");
			Context ctx;
			ctx.GetTmpStore()["PeriodicActionTimeout"] = fWaitTime;
			ctx.Process(fAction);
		}
	}
}
