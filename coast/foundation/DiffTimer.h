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

#if defined(WIN32)
#define HRTIME			__int64
#define GetHRTIME()		GetTickCount()
#define GetHRVTIME()	GetTickCount()
#endif
#ifdef __sun
#include <sys/times.h>
typedef hrtime_t 		HRTIME;
#define GetHRTIME()		gethrtime()
#define GetHRVTIME()	gethrvtime()
#endif
#if defined(__linux__)
#include <time.h>
#include <sys/times.h>
typedef clock_t			HRTIME;
extern "C" HRTIME 		gettimes();
#define GetHRTIME()		gettimes()
#define GetHRVTIME()	gettimes()
#endif
#if defined(_AIX)
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
#define GetHRVTIME()	nanoSecondTime()
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
	/*! Timings are system dependent, resolution is used to scale it to user needs
		small infrastructure class to ease timings of scopes with whatever resolution you like.it uses highest resolution timing apis defined on the platform
		\param resolution the number of ticks you want measure per second; 0 means system dependent clock ticks, 1 means second resolution, 1000 means millisecond resolution etc
	*/
	DiffTimer(long resolution = 1000);

	//!copy constructor for difftimers
	DiffTimer(const DiffTimer &dt);

	//!assignement operator for difftimers
	DiffTimer &operator=(const DiffTimer &dt);

	//!destructor
	~DiffTimer() { }

	/*!difference in ticks per seconds since last start; timer is not reset
		\param simulatedValue defines return value for predictable testcases */
	HRTIME Diff(HRTIME simulatedValue = -1);

	//!resets timer to a new start value
	void Start();

	//!resets timer to a new start value and returns the elapsed ticks since last start
	HRTIME Reset();

	//!returns the platform dependent ticks per second
	static HRTIME TicksPerSecond();

protected:
	friend class DiffTimerTest;

	//!scales the system dependent raw clock tick difference to the resolution defined in the constructor
	HRTIME Scale(HRTIME rawDiff);

	//!keeps last startvalue in system dependent ticks
	HRTIME fStart;

	//!stores resolution in ticks per second
	long fResolution;
};

#endif
