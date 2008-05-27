/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DiffTimer_H
#define _DiffTimer_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include <ctime>
#include <cmath>

#if defined(WIN32)
typedef __int64 HRTIME;
#define GetHRTIME()		GetTickCount()
#elif defined(__sun)
#include <sys/times.h>
typedef hrtime_t 		HRTIME;
#define GetHRTIME()		gethrtime()
#elif defined(__linux__)
#include <time.h>
#include <sys/times.h>
typedef clock_t			HRTIME;
extern "C" HRTIME 		gettimes();
#define GetHRTIME()		gettimes()
#elif defined(_AIX)
#include <time.h>
#include <sys/time.h>
#include <sys/systemcfg.h>

typedef long long int HRTIME;

//! return the current time in nanoseconds
inline HRTIME nanoSecondTime()
{
	timebasestruct_t timeBase;

	read_real_time(&timeBase, sizeof(timeBase));
	time_base_to_time(&timeBase, sizeof(timeBase));

	long long int result = ((long long int)timeBase.tb_high) * 1000000000;
	result += timeBase.tb_low;
	return result;
} // nanoSecondTime

#define GetHRTIME()		nanoSecondTime()
#endif

//---- DiffTimer ----------------------------------------------------------
/*! Timer infrastructure to measure elapsed time in program scopes.
The time is measured by system dependent ticks.
Linux:        100 ticks per second, eg. 10ms steps
Sun  : 1000000000 ticks per second, eg. 1ns steps
WIN32:       1000 ticks per second, eg. 1ms steps
The ticks can be scaled to different resolutions, but the accuraccy depends on the used system timer
*/
class EXPORTDECL_FOUNDATION DiffTimer
{
public:
	typedef HRTIME tTimeType;
	enum eResolution {
		eClockTicks = 0,
		eSeconds = 1,
		eMilliseconds = 1000,
		eMicroseconds = 1000000,
		eNanoseconds = 1000000000
	};

	/*! Timings are system dependent, resolution is used to scale it to user needs
		small infrastructure class to ease timings of scopes with whatever resolution you like.it uses highest resolution timing apis defined on the platform
		\param resolution the number of ticks you want measure per second; 0 means system dependent clock ticks, 1 means second resolution, 1000 means millisecond resolution etc */
	DiffTimer(eResolution resolution = eMilliseconds);

	//!copy constructor for difftimers
	DiffTimer(const DiffTimer &dt);

	//!assignement operator for difftimers
	DiffTimer &operator=(const DiffTimer &dt);

	//!destructor
	~DiffTimer() { }
	/*! difference in ticks per seconds since last start; timer is not reset
		\param simulatedValue defines return value for predictable testcases
		\return time difference in requested resolution */
	tTimeType Diff(tTimeType simulatedValue = -1);

	//!resets timer to a new start value
	void Start();

	//!resets timer to a new start value and returns the elapsed ticks since last start
	tTimeType Reset();

	/*! Get back the tTimeType value since Start() or Reset() was called. Does not touch fStart.
		\return difference from now to start */
	tTimeType RawDiff() const {
		return ( GetHRTIME() - fStart );
	}

	/*! Get platform dependent ticks per second
		\return ticks per second */
	static tTimeType TicksPerSecond();

	/*! Calculate relative difference of two values, eg. a percentage value
		\param hrReference reference value, which is the 100% reference mark
		\param hrCurrent value to compare reference with
		\return relative difference */
	static double RelativeChange(tTimeType hrReference, tTimeType hrCurrent, double precision = 100.0) {
		return std::floor(precision * double(hrReference) / double(hrCurrent)) / precision;
	}

	//!scales the system dependent raw clock tick difference to the resolution defined in the constructor
	static tTimeType Scale(tTimeType rawDiff, eResolution resolution);

protected:
	friend class DiffTimerTest;

	//!keeps last startvalue in system dependent ticks
	tTimeType fStart;

	//!stores resolution in ticks per second
	eResolution fResolution;
};

#endif
