/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TestTimer_H
#define _TestTimer_H

#if defined(WIN32)
#include <windows.h>
#define HRTESTTIME __int64
#define GetHRTESTTIME()	GetTickCount()
#define GetHRVTESTTIME() GetTickCount()
#elif defined(__sun)
#include <sys/times.h>
typedef hrtime_t HRTESTTIME;
#define GetHRTESTTIME()	gethrtime()
#define GetHRVTIME()	gethrvtime()
#elif defined(__linux__)
#include <time.h>
#include <sys/times.h>
typedef clock_t			HRTESTTIME;
extern "C" HRTESTTIME gettimes();
#define GetHRTESTTIME()	gettimes()
#define GetHRVTIME()	gettimes()
#elif defined(_AIX)
#include <time.h>
#include <sys/time.h>
#include <sys/systemcfg.h>

typedef long long int HRTESTTIME;

//! return the current time in nanoseconds
inline HRTESTTIME nanoSecondTime()
{
	timebasestruct_t timeBase;

	read_real_time(&timeBase, sizeof(timeBase));
	time_base_to_time(&timeBase, sizeof(timeBase));

	long long int result = ((long long int)timeBase.tb_high) * 1000000000;
	result += timeBase.tb_low;
	return result;
} // nanoSecondTime

#define GetHRTESTTIME()		nanoSecondTime()
#define GetHRVTIME()	nanoSecondTime()
#endif

//---- TestTimer ----------------------------------------------------------
//! <B>timer infrastructure to measure elapsed time in program scopes. </B>
/*!Timer infrastructure to measure elapsed time in program scopes.
 * The time is measured by system dependent ticks.
 * The ticks can be scaled to different resolutions, but the accuraccy depends on the used system timer
*/
class TestTimer
{
public:
	//! Timings are system dependent, resolution try to scale it to ticks per second
	//! \param resolution the number of ticks you want measure per second; 0 means system dependent clock ticks
	//! small infrastructure class to ease timings of scopes with whatever resolution you like;<br> it uses highest resolution timing apis defined on the platform
	TestTimer(long resolution = 1000);

	//!copy constructor for difftimers
	TestTimer(const TestTimer &dt);

	//!assignement operator for difftimers
	TestTimer &operator=(const TestTimer &dt);

	//!destructor
	~TestTimer() { }

	//!difference in ticks per seconds since last start; timer is not reset
	//! \param simulatedValue defines return value for predictable testcases
	long Diff(long simulatedValue = -1);

	//!resets timer to a new start value
	void Start();

	//!resets timer to a new start value and returns the elapsed ticks since last start
	long Reset();

protected:
	friend class TestTimerTest;

	//!scales the system dependent raw clock tick difference to the resolution defined in the constructor
	long Scale(HRTESTTIME rawDiff);

	//!returns the platform dependent ticks per second
	static HRTESTTIME TicksPerSecond();

	//!keeps last startvalue in system dependent ticks
	HRTESTTIME fStart;

	//!stores resolution in ticks per second
	long fResolution;
};

#endif
