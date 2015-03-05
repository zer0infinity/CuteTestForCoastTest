/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TimeStamp_H
#define _TimeStamp_H

#include "ITOString.h"//lint !e537

//! Timestamps as used by Frontdoor with external representation
/*!
The external representation is "%Y%m%d%H%M%S" relative to timezone GMT.
*/
class TimeStamp
{
	friend class TimeStampTest;
public:
	typedef time_t TSIntNumberType;

	//--- constructors
	//! With time now
	TimeStamp();

	//! With given UTC
	explicit TimeStamp(TSIntNumberType utc);

	//! With given string timestamp
	explicit TimeStamp(const String &externalTimeRep);

	//! With given components
	explicit TimeStamp(char iCent, char iYear, char iMonth, char iDay, char iHour = 0, char iMin = 0, char iSec = 0);

	//! implement copy constructor
	TimeStamp(const TimeStamp &aTimeStamp);

	static TimeStamp Now() {
		return TimeStamp();
	}
	static TimeStamp Min() {
		return TimeStamp(19, 70, 1, 1, 0, 0, 0);
	}
	static TimeStamp Max() {
		return TimeStamp(20, 37, 12, 31, 23, 59, 59);
	}

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
		return fTimeStruct.Century();
	}
	long Year() const {
		return fTimeStruct.Year();
	}
	char YearShort() const {
		return fTimeStruct.YearShort();
	}
	char Month() const {
		return fTimeStruct.Month();
	}
	const char *MonthName() const;
	char Day() const {
		return fTimeStruct.Day();
	}
	const char *DayName() const;
	char Hour() const {
		return fTimeStruct.Hour();
	}
	char Minute() const {
		return fTimeStruct.Minute();
	}
	char Second() const {
		return fTimeStruct.Second();
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
	TimeStamp &operator=(TSIntNumberType lTimeUtc);
	TimeStamp &operator=(String strStamp);
	TimeStamp &Set(const String &externalTimeRep);
	bool SetTime( unsigned char iHour, unsigned char iMin, unsigned char iSec );

	// Output operator
	inline friend std::ostream &operator<< (std::ostream &os, const TimeStamp &ts) {
		os << ts.AsStringWithZ();
		return os;
	}

	static inline bool IsLeap(int iYear) {
		return ((iYear) % 4 == 0 && ((iYear) % 100 != 0 || (iYear) % 400 == 0));
	}

	class AMonth
	{
		AMonth();
	public:
		enum eMonthNumber {
			eJanuary = 1,
			eFebruary = 2,
			eMarch = 3,
			eApril = 4,
			eMay = 5,
			eJune = 6,
			eJuly = 7,
			eAugust = 8,
			eSeptember = 9,
			eOctober = 10,
			eNovember = 11,
			eDecember = 12
		};
		explicit AMonth( eMonthNumber aMonth )
			: fMonth( aMonth ) {}
		TSIntNumberType AsSeconds( unsigned short lInYear = 1970 ) const;
		unsigned char fMonth;
	};
	class AYear
	{
		AYear();
	public:
		explicit AYear( long lYear )
			: fYear( lYear ) {}
		TSIntNumberType AsSeconds() const;
		unsigned short fYear;
	};

	/*! number of seconds per item */
	static const long MIN;
	static const long HOUR;
	static const long DAY;

protected:
	struct intTimeRep {
		enum eItemPositions {
			eCent = 0,
			eYear = 1,
			eMonth = 2,
			eDay = 3,
			eHour = 4,
			eMin = 5,
			eSec = 6,
			eMax = 7
		};
		char cData[eMax];
		unsigned char fStructPos;

		intTimeRep();
		intTimeRep(char iCent, char iYear, char iMonth, char iDay, char iHour, char iMin, char iSec);
		void AddCharacter(char c);
		void Reset();
		String TraceIntValues() const;
		bool IsValidDate() const;
		TSIntNumberType AsTimeT() const;
		String AsString() const;
		bool InitFromTimeT(TSIntNumberType lTime);
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
		char Century() const {
			return cData[eCent];
		}
		long Year() const {
			return (cData[eCent] * 100) + cData[eYear];
		}
		char YearShort() const {
			return cData[eYear];
		}
		char Month() const {
			return cData[eMonth];
		}
		char Day() const {
			return cData[eDay];
		}
		char Hour() const {
			return cData[eHour];
		}
		char Minute() const {
			return cData[eMin];
		}
		char Second() const {
			return cData[eSec];
		}
		bool SetTime( unsigned char iHour, unsigned char iMin, unsigned char iSec );
	};

	// internal initializer
	bool IntSet(intTimeRep aRep);

private:
	bool IntDoInit(const String &externalTimeRep);
	bool SetTimeT(TSIntNumberType lTime);
	String IntTimeTAsString() const;

	intTimeRep fTimeStruct;
};

#endif
