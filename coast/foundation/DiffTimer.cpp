/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "DiffTimer.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

#if defined(__linux__)
HRTIME  gettimes()
{
	struct tms tt;
	return times(&tt);
}
#endif

//---- DiffTimer ---------------------------------------------------------------
DiffTimer::DiffTimer(tTimeType resolution)
	: fResolution(resolution)
{
	StartTrace(DiffTimer.DiffTimer);
	Trace("TicksPerSecond(): " << TicksPerSecond() << " fResolution: " << fResolution);
	if ( fResolution <= 0 ) {
		fResolution = 0;    // measure in clock ticks
	}

	Start();
}

DiffTimer::DiffTimer(const DiffTimer &dt)
	: fResolution(dt.fResolution)
{
	StartTrace(DiffTimer.DiffTimer);
	Trace("TicksPerSecond(): " << TicksPerSecond() << " fResolution: " << fResolution);
	if ( fResolution <= 0 ) {
		fResolution = 0;    // measure in clock ticks
	}

	Start();
}

DiffTimer &DiffTimer::operator=(const DiffTimer &dt)
{
	StartTrace(DiffTimer.operator = );
	fResolution = dt.fResolution;
	return *this;
}

DiffTimer::tTimeType DiffTimer::Scale(tTimeType rawDiff, tTimeType resolution)
{
	StartTrace(DiffTimer.Scale);
	Trace("TicksPerSecond(): " << TicksPerSecond() << " resolution: " << resolution << " rawDiff " << (long)rawDiff);
	if ( resolution <= 0 ) {
		return rawDiff;
	}
	if ( TicksPerSecond() < resolution ) {
		// beware of wrong scale
		return ( rawDiff * ( resolution / TicksPerSecond() ) );
	}
	// beware of overflow
	return ( (rawDiff * resolution) / TicksPerSecond() );
}

DiffTimer::tTimeType DiffTimer::Diff(tTimeType simulatedValue)
{
	StartTrace(DiffTimer.Diff);
	if (simulatedValue > -1) {
		Trace("Using simulated Value: " << simulatedValue);
		return simulatedValue;
	} else {
		tTimeType lDiff = Scale(RawDiff(), fResolution);
		Trace("Diff is: " << (long)lDiff);
		return lDiff;
	}
}

void DiffTimer::Start()
{
	StartTrace(DiffTimer.Start);
	fStart = GetHRTIME();
}

DiffTimer::tTimeType DiffTimer::Reset()
{
	StartTrace(DiffTimer.Reset);
	tTimeType delta = Diff();
	Start();
	return delta;
}

DiffTimer::tTimeType DiffTimer::TicksPerSecond()
{
	StartTrace(DiffTimer.TicksPerSecond);
#if defined(__linux__)
	return sysconf(_SC_CLK_TCK);
#elif defined(__sun)
	return 1000000000;
#elif defined(WIN32)
	return 1000;
#endif
}
