/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TimeStamp_H
#define _TimeStamp_H

#include "config_foundation.h"
#include "ITOString.h"
#include <time.h>

//---- TimeStamp ----------------------------------------------------------
//! <B>Timestamps as used by Frontdoor with external representation</B>
/*!
The external representation is "%Y%m%d%H%M%S" relative to timezone GMT.
*/
class EXPORTDECL_FOUNDATION TimeStamp
{
	friend class TimeStampTest;
public:
	//--- constructors
	//! With time now
	TimeStamp();

	//! With given UTC
	TimeStamp(time_t utc);

	//! With given string timestamp
	TimeStamp(const String &externalTimeRep);

	//! With given components
	TimeStamp(char iCent, char iYear, char iMonth, char iDay, char iHour = 0, char iMin = 0, char iSec = 0);

	//! implement copy constructor
	TimeStamp(const TimeStamp &aTimeStamp);

	static TimeStamp Now() {
		return TimeStamp();
	}
	static TimeStamp Min() {
		return TimeStamp("19700101000000");
	} ;
	static TimeStamp Max() {
		return TimeStamp("20371231235959");
	} ;

	enum eWeekday {
		eSunday = 0,
		eMonday = 1,
		eTuesday = 2,
		eWednesday = 3,
		eThursday = 4,
		eFriday = 5,
		eSaturday = 6
	};

	char Century() const {
		return fTimeStruct.cCent;
	}
	long Year() const {
		return (fTimeStruct.cCent * 100) + fTimeStruct.cYear;
	}
	char YearShort() const {
		return fTimeStruct.cYear;
	}
	char Month() const {
		return fTimeStruct.cMonth;
	}
	char Day() const {
		return fTimeStruct.cDay;
	}
	char Hour() const {
		return fTimeStruct.cHour;
	}
	char Minute() const {
		return fTimeStruct.cMin;
	}
	char Second() const {
		return fTimeStruct.cSec;
	}
	/*! calculate the day of week where Sunday is day 0, Monday day 1 and so on.
		An algorithm description can be found at: http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week
		\return day of week as described by TimeStamp::eWeekday */
	inline eWeekday Weekday() const {
		return fTimeStruct.Weekday();
	}
	/*! return the day number beginning at 1 for the first of january
		\return day number beginning at 1 */
	inline int DayOfYear() const {
		return fTimeStruct.DayOfYear();
	}
	/*! week number calculation according to ISO8601
		\return number of week in the range 0-53 where 0 means that the week number is the last week number of the last year. If the week number is 53 the last day of the year seems to be either a Thursday or a Friday. */
	inline int WeekOfYear() const {
		return fTimeStruct.WeekOfYear();
	}

	String AsString() const;
	String AsStringWithZ() const;

	bool	IsValid() const;
	long	AsLong() const;
	bool	IsBefore(const TimeStamp &) const;
	bool	IsAfter(const TimeStamp &) const;
	bool	IsEqual(const TimeStamp &) const;
	bool	IsNotEqual(const TimeStamp &) const;
	bool	IsAfterEqual(const TimeStamp &) const;
	bool	IsBeforeEqual(const TimeStamp &) const;

	bool	operator>(const TimeStamp &isAfter) const {
		return IsAfter(isAfter);
	};
	bool	operator<(const TimeStamp &isBefore) const {
		return IsBefore(isBefore);
	};
	bool	operator==(const TimeStamp &isEqual) const {
		return IsEqual(isEqual);
	};
	bool	operator!=(const TimeStamp &isNotEqual) const {
		return IsNotEqual(isNotEqual);
	};
	bool	operator>=(const TimeStamp &isAfterEqual) const {
		return IsAfterEqual(isAfterEqual);
	};
	bool	operator<=(const TimeStamp &isBeforeEqual) const {
		return IsBeforeEqual(isBeforeEqual);
	};

	TimeStamp  operator+(long deltasecs) const;
	TimeStamp  operator-(long deltasecs) const;

	TimeStamp &operator+=(long deltasecs);
	TimeStamp &operator-=(long deltasecs);

	TimeStamp &operator=(const TimeStamp &aStamp);
	TimeStamp &operator=(time_t lTimeUtc);
	TimeStamp &operator=(String strStamp);
	TimeStamp &Set(const String &externalTimeRep);

	// Conversion operator, allows String << TimeStamp invocations
	inline operator String () const {
		return AsStringWithZ();
	}
	// Output operator
	inline friend ostream &operator<< (ostream &os, const TimeStamp &ts) {
		os << ts.AsStringWithZ();
		return os;
	}

	/*! number of seconds per item */
	static const long MIN;
	static const long HOUR;
	static const long DAY;
	static const long YEAR;

	static inline bool IsLeap(int iYear) {
		return ((iYear) % 4 == 0 && ((iYear) % 100 != 0 || (iYear) % 400 == 0));
	}

protected:
	struct EXPORTDECL_FOUNDATION intTimeRep {
		char	cCent;
		char	cYear;
		char	cMonth;
		char	cDay;
		char	cHour;
		char	cMin;
		char	cSec;
		char	cTerminator;
		long	fStructPos;

		intTimeRep() {
			Reset();
		}
		intTimeRep(char iCent, char iYear, char iMonth, char iDay, char iHour, char iMin, char iSec)
			: cCent(iCent), cYear(iYear), cMonth(iMonth), cDay(iDay), cHour(iHour), cMin(iMin), cSec(iSec)
			, cTerminator(0), fStructPos(7)
		{ }
		void AddCharacter(char c) {
			long lIdx = (fStructPos >> 1);
			((char *)this)[lIdx] *= 10;
			((char *)this)[lIdx] += c - '0';
			++fStructPos;
		}
		void Reset() {
			fStructPos = 0;
			cTerminator = 0;
			for (long lIdx = 0; lIdx < ((&cTerminator) - (char *)this); ++lIdx) {
				((char *)this)[lIdx] = 0;
			}
		}
		String TraceIntValues() const;
		bool IsValidDate() const;
		time_t AsTimeT() const;
		String AsString() const;
		bool InitFromTimeT(time_t lTime);
		/*! calculate the day of week where Sunday is day 0, Monday day 1 and so on.
			An algorithm description can be found at: http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week
			\return day of week as described by TimeStamp::eWeekday */
		eWeekday Weekday() const;
		/*! return the day number beginning at 1 for the first of january
			\return day number beginning at 1 */
		int DayOfYear() const;
		/*! week number calculation according to ISO8601
			\return number of week in the range 0-53 where 0 means that the week number is the last week number of the last year. If the week number is 53 the last day of the year seems to be either a Thursday or a Friday. */
		int WeekOfYear() const;
	} fTimeStruct;

	// internal initializer
	bool IntSet(intTimeRep aRep);

private:
	String RemoveNonNumericalChars(const String &externalTimeRep);
	bool IntDoInit(const String &externalTimeRep);
	bool SetTimeT(time_t lTime);
	String IntTimeTAsString() const;

	time_t fTime;
	String fRep;
	bool fTimeSet;
};

#endif
