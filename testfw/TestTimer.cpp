/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include -------------------------------------------------------
#include "TestTimer.h"

//--- standard modules used ----------------------------------------------------

//--- c-modules used -----------------------------------------------------------
#if !defined (WIN32)
#include <unistd.h>
#endif

//---- TestTimer ----------------------------------------------------------------
TestTimer::TestTimer(long resolution) : fResolution(resolution)
{
	if ( fResolution <= 0 ) {
		fResolution = 1;    // measure in clock ticks
	}

	Start();
}

TestTimer::TestTimer(const TestTimer &dt) : fResolution(dt.fResolution)
{
	if ( fResolution <= 0 ) {
		fResolution = 1;    // measure in clock ticks
	}

	Start();
}

TestTimer &TestTimer::operator=(const TestTimer &dt)
{
	fResolution = dt.fResolution;
	return *this;
}

HRTESTTIME TestTimer::TicksPerSecond()
{
#if defined(__linux__)
	return sysconf(_SC_CLK_TCK);
#elif defined(__sun)
	return 1000000000;
#elif defined(WIN32)
	return 1000;
#endif
}

long TestTimer::Scale(HRTESTTIME rawDiff)
{
	if ( TicksPerSecond() < fResolution ) {
		// beware of wrong scale
		return (long)(rawDiff * fResolution / TicksPerSecond());
	}
	// beware of overflow
	return (long)((rawDiff * fResolution) / TicksPerSecond());
}

long TestTimer::Diff(long simulatedValue)
{
	if (simulatedValue > -1) {
		return simulatedValue;
	} else {
		long lDiff = Scale(GetHRTESTTIME() - fStart);
		return lDiff;
	}
}

void TestTimer::Start()
{
	fStart = GetHRTESTTIME();
}

long TestTimer::Reset()
{
	long delta = Diff();
	Start();
	return delta;
}
