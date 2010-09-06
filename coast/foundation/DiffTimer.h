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
#else
#include <time.h>
#include <sys/times.h>
typedef clock_t			HRTIME;
extern "C" HRTIME 		gettimes();
#define GetHRTIME()		gettimes()
#endif

//---- DiffTimer ----------------------------------------------------------
//! Timer infrastructure to measure elapsed time in program scopes.
/*! The Timer can be setup to return a scaled result based on the resolution specified when constructing it.
\note The accurracy depends on the system timer used for measurement. Therefor you should not expect more precise results than the appropriate platform default. */
class EXPORTDECL_FOUNDATION DiffTimer
{
public:
	typedef HRTIME tTimeType;

	//! used to specify unit of measurement
	enum eResolution {
		eClockTicks = 0,			/*!< use platform specific maximal resolution, following system dependant maximal values exist:
										- Linux: 100 ticks per second, eg. 10ms steps
										- Sun: 1000000000 ticks per second, eg. 1ns steps
										- Win32: 1000 ticks per second, eg. 1ms steps */
		eSeconds = 1,				//!< seconds resolution
		eMilliseconds = 1000,		//!< milliseconds resolution
		eMicroseconds = 1000000,	//!< microseconds resolution
		eNanoseconds = 1000000000	//!< nanoseconds resolution
	};

	/*! Timings are system dependent, resolution is used to scale it to user needs
		small infrastructure class to ease timings of scopes with whatever resolution you like.it uses highest resolution timing apis defined on the platform
		\param resolution the number of ticks you want measure per second; 0 means system dependent clock ticks, 1 means second resolution, 1000 means millisecond resolution etc */
	DiffTimer(eResolution resolution = eMilliseconds);

	//! copy constructor for difftimers
	DiffTimer(const DiffTimer &dt);

	//! assignement operator for difftimers
	DiffTimer &operator=(const DiffTimer &dt);

	//! destructor
	~DiffTimer() { }

	/*! difference in ticks per seconds since last start; timer is not reset
		\param simulatedValue defines return value for predictable testcases
		\return time difference in requested resolution */
	tTimeType Diff(tTimeType simulatedValue = -1);

	//! starts/resets timer to a new start value
	void Start();

	//! resets timer to a new start value and returns the scaled result - according to used resolution - since last start
	/*! \return scaled result - according to used resolution - since last start */
	tTimeType Reset();

	/*! Retrieve unscaled result since Start() or Reset() was called. Does not touch fStart.
		\return difference from now to start */
	tTimeType RawDiff() const {
		return ( GetHRTIME() - fStart );
	}

	/*! Get platform dependent ticks per second
		\return ticks per second */
	static tTimeType TicksPerSecond();

	/*! Calculate relative difference of two values and scale to given precision, eg. a percentage value
		\param hrReference reference value, which is the 100% reference mark
		\param hrCurrent value to compare reference with
		\param precision value to scale for, 100.0 for example will scale as it was a precentage value
		\return relative difference */
	static double RelativeChange(tTimeType hrReference, tTimeType hrCurrent, double precision = 100.0) {
		return std::floor(precision * double(hrReference) / double(hrCurrent)) / precision;
	}

	//! scales the system dependent raw clock tick difference to objects resolution defined when constructing it
	/*! \param rawDiff unscaled value to scale
		\param resolution scale rawDiff to this resolution
		\return scaled rawDiff result according to resolution */
	static tTimeType Scale(tTimeType rawDiff, eResolution resolution);

protected:
	friend class DiffTimerTest;

	//! keeps last startvalue in system dependent ticks
	tTimeType fStart;

	//! stores resolution in ticks per second
	eResolution fResolution;
};

#endif
