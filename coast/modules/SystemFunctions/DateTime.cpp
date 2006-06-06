/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "DateTime.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SysLog.h"

//--- c-modules used -----------------------------------------------------------
#include <time.h>
#if !defined(WIN32)
#include <sys/time.h>
#endif

#if defined(WIN32)
// the following code was adapted from the ACE-library

#define SECOND_TO_USECS 1000000L
void normalize (timeval *tv_)
{
	// From Hans Rohnert...
	if (tv_->tv_usec >= SECOND_TO_USECS) {
		do {
			tv_->tv_sec++;
			tv_->tv_usec -= SECOND_TO_USECS;
		} while (tv_->tv_usec >= SECOND_TO_USECS);
	} else if (tv_->tv_usec <= -SECOND_TO_USECS) {
		do {
			tv_->tv_sec--;
			tv_->tv_usec += SECOND_TO_USECS;
		} while (tv_->tv_usec <= -SECOND_TO_USECS);
	}

	if (tv_->tv_sec >= 1 && tv_->tv_usec < 0) {
		tv_->tv_sec--;
		tv_->tv_usec += SECOND_TO_USECS;
	} else if (tv_->tv_sec < 0 && tv_->tv_usec > 0) {
		tv_->tv_sec++;
		tv_->tv_usec -= SECOND_TO_USECS;
	}
}

int gettimeofday(timeval *tv_, void *dummy)
{
	StartTrace(DateTime.gettimeofday);
	// Static constant to remove time skew between FILETIME and POSIX
	// time.  POSIX and Win32 use different epochs (Jan. 1, 1970 v.s.
	// Jan. 1, 1601).  The following constant defines the difference
	// in 100ns ticks.
	//
	// In the beginning (Jan. 1, 1601), there was no time and no computer.
	// And Bill said: "Let there be time," and there was time....
	const DWORDLONG FILETIME_to_timval_skew = 0x19db1ded53e8000ui64;

	FILETIME   file_time;
	ULARGE_INTEGER _100ns;
	::GetSystemTimeAsFileTime (&file_time);
	_100ns.LowPart = file_time.dwLowDateTime;
	_100ns.HighPart = file_time.dwHighDateTime;
	_100ns.QuadPart -= FILETIME_to_timval_skew;

	Trace("SystemTime hi:" << (long)_100ns.HighPart << " lo:" << (long)_100ns.LowPart);
	// Convert 100ns units to seconds;
	tv_->tv_sec = (long) (_100ns.QuadPart / (10000 * 1000));
	// Convert remainder to microseconds;
	tv_->tv_usec = (long) ((_100ns.QuadPart % (10000 * 1000)) / 10);
	normalize(tv_);
	return 0;
}
#endif //WIN32

//---- DateTime ----------------------------------------------------------------
void DateTime::GetTimeOfDay(Anything &anyTime, bool bLocalTime)
{
	StartTrace(DateTime.GetTimeOfDay);
	timeval tv;
	// timezone defines difference of local time to utc in seconds
	long lTZ = GetTimezone(), lSecs = 0L;
	anyTime["tzone_sec"] = lTZ;
	if (0 == gettimeofday(&tv, NULL)) {
		anyTime["usec"] = tv.tv_usec;
		lSecs = tv.tv_sec - (bLocalTime ? lTZ : 0L);
	} else {
		// fall back towards using time
		lSecs = time(0) - (bLocalTime ? lTZ : 0L);
		SYSWARNING("Fallback to time() function! No usec available!");
	}
	// store seconds since 1.1.1970
	anyTime["sec"] = lSecs;
	// prepare for seconds in current day
	lSecs = lSecs % 86400;
	anyTime["sec_since_midnight"] = lSecs;
	anyTime["msec_since_midnight"] = (lSecs * 1000) + (anyTime["usec"].AsLong(0L) / 1000);
	TraceAny(anyTime, "current time");
}

long DateTime::GetTimezone()
{
	StartTrace(DateTime.GetTimeZone);
	// timezone defines difference of local time to utc in seconds
	tzset();
	return timezone;
}
