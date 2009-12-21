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
#include "boost/format.hpp"

static DiffTimer::eResolution ullResolution = DiffTimer::eMicroseconds;

//---- WPMStatHandler ----------------------------------------------------------------
WPMStatHandler::WPMStatHandler(long poolSize)
	: StatEvtHandler()
	, fPoolSize(poolSize)
	, fMaxParallelRequests(0)
	, fCurrentParallelRequests(0)
	, fTotalRequests(0)
	, fTotalTime(0)
	, fTimer( ullResolution )
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
	ul_long ullTotalTime, ullTotalRequests;
	static double dResolution( ullResolution );
	{
		LockUnlockEntry me(fMutex);
		statElements["PoolSize"] = fPoolSize;
		statElements["CurrentParallelRequests"] = fCurrentParallelRequests;
		statElements["MaxParallelRequests"] = fMaxParallelRequests;
		ullTotalTime = fTotalTime;
		ullTotalRequests = fTotalRequests;
	}
	statElements["TotalRequests"] = (long)ullTotalRequests;
	// scale to milliseconds
	statElements["TotalTime [ms]"] = (long)( ullTotalTime / 1000ULL );
	boost::format avgFmt("%-0.1f"), trxFmt("%-0.1f");
	double dTotalTime( ullTotalTime ), dTotalRequests( ullTotalRequests );
	statElements["AverageTime [ms]"] = (ullTotalRequests ? ( avgFmt % ( dTotalTime / dTotalRequests / 1000.0 ) ).str().c_str() : "0");
	statElements["TRX/sec"] = (ullTotalRequests ? (trxFmt % ( dResolution * ( dTotalRequests / dTotalTime ) ) ).str().c_str() : "0");
	TraceAny(statElements, "statElements");
}

long WPMStatHandler::DoGetTotalRequests()
{
	LockUnlockEntry me(fMutex);
	StatTrace(WPMStatHandler.DoGetTotalRequests, "total: " << (l_long)fTotalRequests, Storage::Current());
	return fTotalRequests;
}

long WPMStatHandler::DoGetCurrentParallelRequests()
{
	LockUnlockEntry me(fMutex);
	StatTrace(WPMStatHandler.DoGetCurrentParallelRequests, "curr: " << fCurrentParallelRequests, Storage::Current());
	return fCurrentParallelRequests;
}
