/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DateTime_H
#define _DateTime_H

//---- include -------------------------------------------------
#include "Anything.h"

//---- DateTime ----------------------------------------------------------
//! wrapper class for date and time functions
/*!
*/
class DateTime
{
public:
	//--- public api
	/*! wrapper function for gettimeofday
		\param anyTime structure to hold returned values
		\param bLocalTime if set to false, UTC(GMT) time will be returned
		<b>returned elements:</b>
		<pre>{
			/sec		long	seconds since 1.1.1970
			/usec		long	microseconds of current second
			/tzone_sec	long	timezone specific difference to above values in seconds
			/sec_since_midnight	long	seconds since midnight
			/msec_since_midnight	long	milliseconds since midnight
		}</pre> */
	static void GetTimeOfDay(Anything &anyTime, bool bLocalTime = true);

	/*! wrapper function for getting timezone difference in seconds
		\return	difference in seconds */
	static long GetTimezone();

private:
	//--- constructors
	DateTime();
	~DateTime();
};

#endif
