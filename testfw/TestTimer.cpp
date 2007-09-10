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
TestTimer::TestTimer(tTimeType resolution)
	: fResolution(resolution)
{
	if ( fResolution <= 0 ) {
		fResolution = 0;    // measure in clock ticks
	}

	Start();
}

TestTimer::TestTimer(const TestTimer &dt)
	: fResolution(dt.fResolution)
{
	if ( fResolution <= 0 ) {
		fResolution = 0;    // measure in clock ticks
	}

	Start();
}

TestTimer &TestTimer::operator=(const TestTimer &dt)
{
	fResolution = dt.fResolution;
	return *this;
}

TestTimer::tTimeType TestTimer::Scale(tTimeType rawDiff, tTimeType resolution)
{
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

TestTimer::tTimeType TestTimer::Diff(tTimeType simulatedValue)
{
	if (simulatedValue > -1) {
		return simulatedValue;
	} else {
		tTimeType lDiff = Scale(RawDiff(), fResolution);
		return lDiff;
	}
}

void TestTimer::Start()
{
	fStart = GetHRTESTTIME();
}

TestTimer::tTimeType TestTimer::Reset()
{
	tTimeType delta = Diff();
	Start();
	return delta;
}

TestTimer::tTimeType TestTimer::TicksPerSecond()
{
#if defined(__linux__)
	return sysconf(_SC_CLK_TCK);
#elif defined(__sun)
	return 1000000000;
#elif defined(WIN32)
	return 1000;
#endif
}
