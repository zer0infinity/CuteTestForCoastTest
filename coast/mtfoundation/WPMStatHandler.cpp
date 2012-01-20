/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "WPMStatHandler.h"
#include "boost/format.hpp"

namespace
{
	DiffTimer::eResolution ullResolution = DiffTimer::eMicroseconds;
};

WPMStatHandler::WPMStatHandler(long poolSize)
	: StatEvtHandler()
	, fPoolSize(poolSize)
	, fMaxParallelRequests(0)
	, fCurrentParallelRequests(0)
	, fTotalRequests(0)
	, fTotalTime(0.0)
	, fTimer( ullResolution )
	, fMutex( "WPMStatHandler", Coast::Storage::Global() )
{
	StartTrace(WPMStatHandler.Ctor);
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
	ul_long ullTotalRequests(0LL);
	double dTotalTime(0.0);
	{
		LockUnlockEntry me(fMutex);
		statElements["PoolSize"] = fPoolSize;
		statElements["CurrentParallelRequests"] = fCurrentParallelRequests;
		statElements["MaxParallelRequests"] = fMaxParallelRequests;
		dTotalTime = fTotalTime;
		if ( fCurrentParallelRequests > 0 ) {
			// need to account for elapsed time when the pool is currently under load
			dTotalTime += fTimer.Diff();
		}
		ullTotalRequests = fTotalRequests;
	}
	statElements["TotalRequests"] = (long)ullTotalRequests;
	// scale from microseconds to milliseconds
	boost::format avgFmt("%-0.1f"), trxFmt("%-0.1f"), totFmt("%-0.1f");
	double dTotalRequests( ullTotalRequests ),
		   dScaleResolutionToMillisecondsFactor = (static_cast<double>(DiffTimer::eMilliseconds) / static_cast<double>(ullResolution)),
		   dTrxPusec = ( dTotalTime > 0.0 ? ( dTotalRequests / dTotalTime ) : 0.0 ),
		   dAvgTimemsec = ( dTrxPusec > 0.0 ? ( dScaleResolutionToMillisecondsFactor / dTrxPusec ) : 0.0 ),
		   dTrxPSec = ( dTotalTime > 0.0 ? ( 1 / dAvgTimemsec * static_cast<double>(DiffTimer::eMilliseconds) / static_cast<double>(DiffTimer::eSeconds)) : 0.0 );
	statElements["TotalTime [ms]"] = ( ( dTotalTime * dScaleResolutionToMillisecondsFactor ) > 0.0 ? (totFmt % ( dTotalTime * dScaleResolutionToMillisecondsFactor )).str().c_str() : "0" );
	statElements["AverageTime [ms]"] = (ullTotalRequests ? ( avgFmt % dAvgTimemsec ).str().c_str() : "0");
	statElements["TRX/sec"] = (ullTotalRequests ? ( trxFmt % dTrxPSec ).str().c_str() : "0");
	TraceAny(statElements, "statElements");
}

long WPMStatHandler::DoGetTotalRequests()
{
	LockUnlockEntry me(fMutex);
	StatTrace(WPMStatHandler.DoGetTotalRequests, "total: " << (l_long)fTotalRequests, Coast::Storage::Current());
	return fTotalRequests;
}

long WPMStatHandler::DoGetCurrentParallelRequests()
{
	LockUnlockEntry me(fMutex);
	StatTrace(WPMStatHandler.DoGetCurrentParallelRequests, "curr: " << fCurrentParallelRequests, Coast::Storage::Current());
	return fCurrentParallelRequests;
}
