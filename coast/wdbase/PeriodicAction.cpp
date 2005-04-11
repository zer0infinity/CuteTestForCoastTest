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
	while ( IsRunning() ) {
		Trace("Waiting " << fWaitTime << "s for next period");
		// wait for the next cycle due
		// or the termination signal
		if ( !CheckState(Thread::eTerminationRequested, fWaitTime) && IsRunning() ) {
			Trace("Starting work and calling Action [" << fAction << "]");
			Context ctx;
			ctx.GetTmpStore()["PeriodicActionTimeout"] = fWaitTime;
			ctx.Process(fAction);
			if ( IsRunning() ) {
				Trace("signalling Ready state");
				SetReady();
			}
		}
	}
}
