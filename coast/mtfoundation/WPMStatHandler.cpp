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
	, fMutex( "WPMStatHandler", Storage::Global() )
{
	StartTrace(WPMStatHandler.Ctor);
}

WPMStatHandler::~WPMStatHandler()
{
	StartTrace(WPMStatHandler.Dtor);
}

void WPMStatHandler::DoHandleStatEvt(long evt)
{
	StartTrace1(WPMStatHandler.DoHandleStatEvt, "Event[" << evt << "]");
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
			Trace("eEnter: curr: " << fCurrentParallelRequests << " max: " << fMaxParallelRequests);
		}
		break;

		case eLeave: {
			--fCurrentParallelRequests;
			if ( fCurrentParallelRequests == 0 ) {
				fTotalTime += fTimer.Reset();
			}
			++fTotalRequests;
			Trace("eLeave: curr: " << fCurrentParallelRequests << " max: " << fMaxParallelRequests);
		}
		break;

		default:
			break;
	}
}

void WPMStatHandler::DoStatistic(Anything &statElements)
{
	StartTrace(WPMStatHandler.DoStatistic);
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

long WPMStatHandler::DoGetTotalRequests()
{
	LockUnlockEntry me(fMutex);
	StatTrace(WPMStatHandler.DoGetTotalRequests, "total: " << fTotalRequests, Storage::Current());
	return fTotalRequests;
}

long WPMStatHandler::DoGetCurrentParallelRequests()
{
	LockUnlockEntry me(fMutex);
	StatTrace(WPMStatHandler.DoGetCurrentParallelRequests, "curr: " << fCurrentParallelRequests, Storage::Current());
	return fCurrentParallelRequests;
}
