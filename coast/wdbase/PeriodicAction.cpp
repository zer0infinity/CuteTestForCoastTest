/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Context.h"
#include "Action.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "PeriodicAction.h"

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

		Trace("Waiting for next period");
		// wait for the next cycle due
		// or the termination signal
		CheckRunningState(Thread::eWorking, fWaitTime);
		Trace("Starting work");

		// check the alive flag
		if (!IsRunning()) {
			return;
		} else {

			Trace("calling " << fAction);
			Context ctx;
			ctx.Process(fAction);
			SetReady();
		}
	}
}
