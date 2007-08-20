/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "WPMStatHandler.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"

//---- WPMStatHandler ----------------------------------------------------------------
WPMStatHandler::WPMStatHandler(long poolSize)
	: StatEvtHandler()
	, fPoolSize(poolSize)
	, fMaxParallelRequests(0)
	, fCurrentParallelRequests(0)
	, fTotalRequests(0)
	, fTotalTime(0)
	, fTimer()
	, fMutex("WPMStatHandler")
{
	StartTrace(WPMStatHandler.Ctor);
}

WPMStatHandler::~WPMStatHandler()
{
	StartTrace(WPMStatHandler.Dtor);
}

void WPMStatHandler::HandleStatEvt(long evt)
{
	StartTrace1(WPMStatHandler.HandleStatEvt, "Event[" << evt << "]");
	LockUnlockEntry me(fMutex);

	switch (evt) {
		case eEnter: {
			if ( fCurrentParallelRequests == 0 ) {
				fTimer.Start();
			}
			++fCurrentParallelRequests;
			if (fMaxParallelRequests < fCurrentParallelRequests) {
				fMaxParallelRequests = fCurrentParallelRequests;
			}
			Trace("eEnter: para:[" << fCurrentParallelRequests << "] maxpara[" << fMaxParallelRequests << "]");
		}
		break;

		case eLeave: {
			--fCurrentParallelRequests;
			if ( fCurrentParallelRequests == 0 ) {
				fTotalTime += fTimer.Reset();
			}
			++fTotalRequests;
		}
		break;

		default:
			break;
	}
}

void WPMStatHandler::Statistic(Anything &statElements)
{
	StartTrace(WPMStatHandler.Statistic);
	LockUnlockEntry me(fMutex);
	statElements["PoolSize"] = fPoolSize;
	statElements["CurrentParallelRequests"] = fCurrentParallelRequests;
	statElements["MaxParallelRequests"] = fMaxParallelRequests;
	statElements["TotalRequests"] = fTotalRequests;
	statElements["TotalTime"] = fTotalTime;
	statElements["AverageTime"] = fTotalTime / ((fTotalRequests) ? fTotalRequests : 1);
	statElements["TRX/sec"] = 1000 * fTotalRequests / ((fTotalTime) ? fTotalTime : 1);
	TraceAny(statElements, "statElements");
}

long WPMStatHandler::GetTotalRequests()
{
	StartTrace(WPMStatHandler.GetTotalRequests);
	LockUnlockEntry me(fMutex);
	return fTotalRequests;
}

long WPMStatHandler::GetCurrentParallelRequests()
{
	StartTrace(WPMStatHandler.GetCurrentParallelRequests);
	LockUnlockEntry me(fMutex);
	return fCurrentParallelRequests;
}
