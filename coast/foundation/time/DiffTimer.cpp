/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DiffTimer.h"
#include "SystemBase.h"
#include "Tracer.h"
#if !defined(WIN32)
HRTIME  gettimes()
{
	struct tms tt;
	return times(&tt);
}
#endif

//---- DiffTimer ---------------------------------------------------------------
DiffTimer::DiffTimer(DiffTimer::eResolution resolution)
	: fStart(0),
	  fResolution(resolution)
{
	StartTrace(DiffTimer.DiffTimer);
	Trace("TicksPerSecond(): " << TicksPerSecond() << " fResolution: " << fResolution);
	if ( fResolution <= 0 ) {
		fResolution = eClockTicks;    // measure in clock ticks
	}

	Start();
}

DiffTimer::DiffTimer(const DiffTimer &dt)
	: fStart(dt.fStart),
	  fResolution(dt.fResolution)
{
	StartTrace(DiffTimer.DiffTimer);
	Trace("TicksPerSecond(): " << TicksPerSecond() << " fResolution: " << fResolution);
	if ( fResolution <= 0 ) {
		fResolution = eClockTicks;    // measure in clock ticks
	}

	Start();
}

DiffTimer &DiffTimer::operator=(const DiffTimer &dt)
{
	StartTrace(DiffTimer.operator = );
	fStart = dt.fStart;
	fResolution = dt.fResolution;
	return *this;
}//lint !e1529

DiffTimer::tTimeType DiffTimer::Scale(tTimeType rawDiff, DiffTimer::eResolution resolution)
{
	tTimeType scaled( rawDiff );
	if ( resolution > 0 ) {
		if ( TicksPerSecond() < resolution ) {
			// beware of wrong scale
			scaled = ( rawDiff * ( resolution / TicksPerSecond() ) );
		} else {
			// beware of overflow
			scaled = ( (rawDiff * resolution) / TicksPerSecond() );
		}
	}
	StatTrace(DiffTimer.Scale, "TicksPerSecond(): " << TicksPerSecond() << " resolution: " << resolution << " rawDiff: " << rawDiff << " scaled: " << scaled, Coast::Storage::Current());
	return scaled;
}

DiffTimer::tTimeType DiffTimer::Diff(tTimeType simulatedValue)
{
	StartTrace(DiffTimer.Diff);
	if (simulatedValue > -1) {
		Trace("Using simulated Value: " << simulatedValue);
		return simulatedValue;
	} else {
		tTimeType lDiff = Scale(RawDiff(), fResolution);
		Trace("Diff is: " << static_cast<long>(lDiff));
		return lDiff;
	}
}

void DiffTimer::Start()
{
	fStart = GetHRTIME();
	StatTrace(DiffTimer.Start, "now: " << fStart, Coast::Storage::Current());
}

DiffTimer::tTimeType DiffTimer::Reset()
{
	tTimeType delta = Diff();
	Start();
	StatTrace(DiffTimer.Reset, "delta: " << delta, Coast::Storage::Current());
	return delta;
}

DiffTimer::tTimeType DiffTimer::TicksPerSecond()
{
	tTimeType tps( 1 );
#if defined(__sun)
	tps = 1000000000;
#elif defined(WIN32)
	tps = 1000;
#else
	tps = sysconf(_SC_CLK_TCK);
#endif
	StatTrace(DiffTimer.TicksPerSecond, "tps: " << tps, Coast::Storage::Current());
	return tps;
}
