/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TimeStamp.h"
#include "Tracer.h"
#include "SystemBase.h"
#include "SystemLog.h"

const long TimeStamp::MIN = 60L;
const long TimeStamp::HOUR = TimeStamp::MIN * 60L;
const long TimeStamp::DAY = 24 * TimeStamp::HOUR;
const long YEAR = 365 * TimeStamp::DAY;

static const long MDAYS [] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const int WeekDayCenturyCorrect[] = { 4, 2, 0, 6 };
static const int WeekDayMonthCorrectNoLeap[] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };
static const int WeekDayMonthCorrectLeap[] = { 6, 2, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };
static const char *Monthes[] = { "---", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static const char *Days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

TimeStamp::TimeStamp():fTimeStruct()
{
	StartTrace1(TimeStamp.TimeStamp, "empty");
	SetTimeT(time(0));
}

TimeStamp::TimeStamp(const TimeStamp &aTimeStamp):fTimeStruct(aTimeStamp.fTimeStruct)
{
	StartTrace1(TimeStamp.TimeStamp, "copy [" << aTimeStamp.AsString() << "]");
	//operator=(aTimeStamp);
}

//! With given UTC
TimeStamp::TimeStamp(TSIntNumberType utc)
{
	StartTrace1(TimeStamp.TimeStamp, "time_t:" << utc);
	SetTimeT(utc);
}

TimeStamp::TimeStamp(const String &externalTimeRep)
{
	StartTrace1(TimeStamp.TimeStamp, "String [" << externalTimeRep << "]");
	IntDoInit(externalTimeRep);
}

TimeStamp::TimeStamp(char iCent, char iYear, char iMonth, char iDay, char iHour, char iMin, char iSec)
{
	StartTrace1(TimeStamp.TimeStamp, "components [" << static_cast<long>(iCent) << ':' << static_cast<long>(iYear) << ':' << static_cast<long>(iMonth) << ':' << static_cast<long>(iDay) << ':' << static_cast<long>(iHour) << ':' << static_cast<long>(iMin) << ':' << static_cast<long>(iSec) << "]");
	IntSet(TimeStamp::intTimeRep(iCent, iYear, iMonth, iDay, iHour, iMin, iSec));
}

bool TimeStamp::IntSet(TimeStamp::intTimeRep aRep)
{
	StartTrace1(TimeStamp.IntSet, "intTimeRep [" << aRep.AsString() << "]");
	if ( !aRep.IsValidDate() ) {
		SYSWARNING("invalid input format [" << aRep.AsString() << "]");
		return false;
	} else {
		fTimeStruct = aRep;
		Trace("new time:" << fTimeStruct.AsTimeT());
	}
	return true;
}

TimeStamp &TimeStamp::Set(const String &externalTimeRep)
{
	StartTrace(TimeStamp.Set);
	IntDoInit(externalTimeRep);
	return *this;
}

bool TimeStamp::SetTime( unsigned char iHour, unsigned char iMin, unsigned char iSec )
{
	StartTrace(TimeStamp.SetTime);
	return fTimeStruct.SetTime( iHour, iMin, iSec );
}

bool TimeStamp::IntDoInit(const String &externalTimeRep)
{
	StartTrace1(TimeStamp.IntDoInit, "string to convert [" << externalTimeRep << "]");
	fTimeStruct.Reset();
	const long lMaxDateLen( 14L );
	long lExtLen( externalTimeRep.Length() ), lExtCount(0L), lAddCount(0L);
	const char pattern[] = "19700101000000";
	Trace("extlen:" << lExtLen);
	for (long i = 0; i < lMaxDateLen && lAddCount < lMaxDateLen; ++i) {
		if ( i < lExtLen ) {
			if ( isdigit( externalTimeRep.At(i) ) ) {
				fTimeStruct.AddCharacter( externalTimeRep.At(i) );
				++lExtCount;
				++lAddCount;
			}
		} else {
			// need to fill up internal data with reasonable default of pattern
			fTimeStruct.AddCharacter( pattern[lAddCount] );
			++lAddCount;
		}
	}
	if ( lExtCount < 4L ) {
		SYSWARNING("invalid input format [" << externalTimeRep << "], at least a year (YYYY) must be specified");
		fTimeStruct.Reset();
		return false;
	} else if ( !fTimeStruct.IsValidDate() ) {
		SYSWARNING("invalid input format [" << externalTimeRep << "]");
		return false;
	}
	Trace("valid parsed date " << fTimeStruct.AsString());
	Trace("new time:" << fTimeStruct.AsTimeT());
	return true;
}

bool TimeStamp::SetTimeT(TSIntNumberType lTime)
{
	StartTrace(TimeStamp.SetTimeT);
	bool bRet = false;
	fTimeStruct.Reset();
	if ( lTime >= 0 ) {
		bRet = fTimeStruct.InitFromTimeT(lTime);
	}
	return bRet;
}

long TimeStamp::AsLong() const
{
	StartTrace(TimeStamp.AsLong);
	return fTimeStruct.AsTimeT();
}

bool TimeStamp::IsValid() const
{
	StartTrace(TimeStamp.IsValid);
	return fTimeStruct.IsValidDate();
}

bool TimeStamp::IsBefore(const TimeStamp &isBefore) const
{
	StartTrace(TimeStamp.IsBefore);
	return AsLong() < isBefore.AsLong();
}

bool TimeStamp::IsAfter(const TimeStamp &isAfter) const
{
	StartTrace(TimeStamp.IsAfter);
	return AsLong() > isAfter.AsLong();
}

bool TimeStamp::IsBeforeEqual(const TimeStamp &isBeforeEqual) const
{
	StartTrace(TimeStamp.IsBeforeEqual);
	return AsLong() <= isBeforeEqual.AsLong();
}

bool TimeStamp::IsAfterEqual(const TimeStamp &isAfterEqual) const
{
	StartTrace(TimeStamp.IsAfterEqual);
	return AsLong() >= isAfterEqual.AsLong();
}

bool TimeStamp::IsEqual(const TimeStamp &isEqual) const
{
	StartTrace(TimeStamp.IsEqual);
	return AsLong() == isEqual.AsLong();
}

bool TimeStamp::IsNotEqual(const TimeStamp &isNotEqual) const
{
	StartTrace(TimeStamp.IsNotEqual);
	return !IsEqual(isNotEqual);
}

String TimeStamp::AsString() const
{
	StartTrace(TimeStamp.AsString);
	return IntTimeTAsString();
}

String TimeStamp::AsStringWithZ() const
{
	StartTrace(TimeStamp.AsStringWithZ);
	return AsString() << "Z";
}

String TimeStamp::IntTimeTAsString() const
{
	StartTrace(TimeStamp.IntTimeTAsString);
	// result has length 14, optimize buffer to hold 14 characters plus 0
	String result(16L);
	if ( IsValid() ) {
		result = fTimeStruct.AsString();
	}
	return result;
}

TimeStamp TimeStamp::operator+(long deltasecs) const
{
	StartTrace(TimeStamp.operator + );
	return TimeStamp((TSIntNumberType)(AsLong() + deltasecs));
}

TimeStamp TimeStamp::operator-(long deltasecs) const
{
	StartTrace(TimeStamp.operator - );
	return TimeStamp((TSIntNumberType)(AsLong() - deltasecs));
}

TimeStamp &TimeStamp::operator=(const TimeStamp &aStamp)
{
	StartTrace1(TimeStamp.operator = , "TimeStamp");
	fTimeStruct = aStamp.fTimeStruct;
	return *this;
}//lint !e1529

TimeStamp &TimeStamp::operator=(TSIntNumberType lTimeUtc)
{
	StartTrace1(TimeStamp.operator = , "time_t");
	SetTimeT(lTimeUtc);
	return *this;
}

TimeStamp &TimeStamp::operator=(String strStamp)
{
	StartTrace1(TimeStamp.operator = , "String");
	return Set(strStamp);
}

TimeStamp &TimeStamp::operator+=(long deltasecs)
{
	StartTrace1(TimeStamp.operator += , "long:" << deltasecs);
	SetTimeT(AsLong() + deltasecs);
	return *this;
}

TimeStamp &TimeStamp::operator-=(long deltasecs)
{
	StartTrace1(TimeStamp.operator -= , "long:" << deltasecs);
	return operator+=(-deltasecs);
}

TimeStamp::intTimeRep::intTimeRep()
{
	Reset();
}

TimeStamp::intTimeRep::intTimeRep(char iCent, char iYear, char iMonth, char iDay, char iHour, char iMin, char iSec)
{
	cData[eCent] = iCent;
	cData[eYear] = iYear;
	cData[eMonth] = iMonth;
	cData[eDay] = iDay;
	cData[eHour] = iHour;
	cData[eMin] = iMin;
	cData[eSec] = iSec;
	fStructPos = eMax;
}

void TimeStamp::intTimeRep::Reset()
{
	cData[eCent] = 0;
	cData[eYear] = 0;
	cData[eMonth] = 0;
	cData[eDay] = 0;
	cData[eHour] = 0;
	cData[eMin] = 0;
	cData[eSec] = 0;
	fStructPos = eCent;
}

bool TimeStamp::intTimeRep::SetTime( unsigned char iHour, unsigned char iMin, unsigned char iSec )
{
	StartTrace(TimeStamp.intSetTime);
	bool bSuccess( false );
	if ( ( iHour <= 23 ) && ( iMin <= 59 ) && ( iSec <= 59 ) ) {
		cData[eHour] = iHour;
		cData[eMin] = iMin;
		cData[eSec] = iSec;
		bSuccess = true;
	}
	return bSuccess;
}

void TimeStamp::intTimeRep::AddCharacter(char c)
{
	// the code calculates correct representation number using two characters of input text
	// example input: 2007
	//  '2' -> cData[eCent]=2
	//  '0' -> cData[eCent]=2*10 + 0
	//  '0' -> cData[eYear]=0
	//  '7' -> cData[eYear]=0*10 + 7
	//  ...
	// use the same field twice by dividing index by 2
	eItemPositions aPos( (eItemPositions)( fStructPos >> 1 ) );
	const char cNull = '0';
	cData[aPos] *= 10;
	cData[aPos] += c - cNull;
	++fStructPos;
}

bool TimeStamp::intTimeRep::IsValidDate() const
{
	long year = (cData[eCent] * 100) + cData[eYear];
	bool bRet = true;
	// sanity check, return false if errornous
	if (year < 1970L || year > 2037L ||
		cData[eMonth] < 1 || cData[eMonth] > 12 ||
		cData[eDay] < 1 || cData[eDay] > 31 ||
		cData[eHour] < 0 || cData[eHour] > 23 ||
		cData[eMin] < 0 || cData[eMin] > 59 ||
		cData[eSec] < 0 || cData[eSec] > 59 ||
		( 2 == cData[eMonth] && cData[eDay] > ( TimeStamp::IsLeap(year) ? 29 : 28)) ||
		( (4 == cData[eMonth] || 6 == cData[eMonth] || 9 == cData[eMonth] || 11 == cData[eMonth]) && cData[eDay] > 30) ) {
		bRet = false;
	}
	StatTrace(TimeStamp.IsValidDate, "Date is " << (bRet ? "" : "not ") << "OK " << (bRet ? "" : (const char *)TraceIntValues()), coast::storage::Current());
	return bRet;
}

String TimeStamp::intTimeRep::TraceIntValues() const
{
	String result;
	result.Append("Century:").Append((long)cData[eCent]).Append(" Year:").Append((long)cData[eYear]).Append(" Month:").Append((long)cData[eMonth]).Append(" Day:").Append((long)cData[eDay]).Append(" Hour:").Append((long)cData[eHour]).Append(" Min:").Append((long)cData[eMin]).Append(" Sec:").Append((long)cData[eSec]).Append(" Pos:").Append(static_cast<long>(fStructPos));
	return result;
}

// zero fill number
static char ChrNumbers[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
#define ZFNUM(result, num) if ( num <= 9 ) result.Append('0').Append( ChrNumbers[(int)num] ); else result.Append( ChrNumbers[(num/10)] ).Append( ChrNumbers[(int)num%10] );

String TimeStamp::intTimeRep::AsString() const
{
	// result has length 14, optimize buffer to hold 14 characters plus 0
	String result(15);
	// assemble string
	long year = (cData[eCent] * 100) + cData[eYear];
	result.Append(year);
	ZFNUM(result, cData[eMonth]);
	ZFNUM(result, cData[eDay]);
	ZFNUM(result, cData[eHour]);
	ZFNUM(result, cData[eMin]);
	ZFNUM(result, cData[eSec]);
	StatTrace(TimeStamp.AsString, "intTimeRep [" << TraceIntValues() << "] result [" << result << "]", coast::storage::Current());
	return result;
}

#define LYFA(theyear,mindex) ((1==mindex&&TimeStamp::IsLeap(theyear))?1:0)

TimeStamp::TSIntNumberType TimeStamp::AMonth::AsSeconds( unsigned short lInYear ) const
{
	return ( ( MDAYS[fMonth - 1] + LYFA( lInYear, ( fMonth - 1 ) ) ) * TimeStamp::DAY );
}

TimeStamp::TSIntNumberType TimeStamp::AYear::AsSeconds() const
{
	return ( YEAR + ( TimeStamp::IsLeap( fYear ) ? TimeStamp::DAY : 0L ) );
}

TimeStamp::TSIntNumberType TimeStamp::intTimeRep::AsTimeT() const
{
	TSIntNumberType lTime = 0;
	if ( IsValidDate() ) {
		StartTrace(TimeStamp.AsTimeT);
		long year = (cData[eCent] * 100) + cData[eYear];
		// now calc the seconds
		lTime = (year - 1970) * YEAR;
		Trace("number of leap years to account for:" << ((year - 1) / 4 - 1969 / 4));
		lTime += (((year - 1) / 4 - 1969 / 4)) * TimeStamp::DAY;
		// count leap years before current year, since 1970
		for (int i = 0; i < cData[eMonth] - 1; ++i) {
			lTime += MDAYS[i] * TimeStamp::DAY;
		}
		// leap-year adjustment
		if ( cData[eMonth] > 2 && TimeStamp::IsLeap(year) ) {
			Trace("adjusting current leap year");
			lTime += TimeStamp::DAY;
		}
		Trace("adding days:" << cData[eDay] - 1);
		lTime += (cData[eDay] - 1) * TimeStamp::DAY;
		lTime += cData[eHour] * TimeStamp::HOUR + cData[eMin] * TimeStamp::MIN + cData[eSec];
		Trace("new time:" << lTime);
	}
	return lTime;
}

bool TimeStamp::intTimeRep::InitFromTimeT(TSIntNumberType lTime)
{
	StartTrace(TimeStamp.InitFromTimeT);
	// normal case, do a brain dead standalone conversion, which should be fast!
	long days = lTime / TimeStamp::DAY;
	long rem = lTime % TimeStamp::DAY;
	cData[eHour] = (char)(rem / TimeStamp::HOUR);
	rem %= TimeStamp::HOUR;
	cData[eMin] = (char)(rem / 60);
	cData[eSec] = rem % 60;
	long y = 1970;
	long lYearDays = 0L;
	// count correct year, very simply
	while ( days >= ( lYearDays = (TimeStamp::IsLeap(y) ? 366 : 365) ) ) {
		// could be optimized but runs max about 80 times...
		days -= lYearDays;
		++y;
	}
	cData[eCent] = y / 100;
	cData[eYear] = y % 100;
	// calculate month
	int i = 0;
	// Leap Year February Adjust
	for (i = 0; i < 12; ++i) {
		if (days >= MDAYS[i] + LYFA(y, i)) {
			days -= MDAYS[i] + LYFA(y, i);
		} else {
			break;
		}
	}
	cData[eMonth] = i + 1;
	days += 1;
	cData[eDay] = (char)days;
	return IsValidDate();
}

TimeStamp::eWeekday TimeStamp::intTimeRep::Weekday() const
{
	StartTrace(TimeStamp.Weekday);
	int y = (cData[eCent] * 100) + cData[eYear];
	Trace("isleap:" << (TimeStamp::IsLeap(y) ? "true" : "false"));
	int DoW = WeekDayCenturyCorrect[cData[eCent] - 17];
	DoW += cData[eYear];
	DoW += (cData[eYear] / 4);
	DoW += (TimeStamp::IsLeap(y) ? WeekDayMonthCorrectLeap[cData[eMonth] - 1] : WeekDayMonthCorrectNoLeap[cData[eMonth] - 1]);
	DoW += cData[eDay];
	DoW = (DoW % 7);
	Trace("DoW:" << DoW);
	return (TimeStamp::eWeekday)DoW;
}

int TimeStamp::intTimeRep::DayOfYear() const
{
	StartTrace(TimeStamp.DayOfYear);
	int lDay = 0;
	for (int i = 0; i < cData[eMonth] - 1; ++i) {
		lDay += MDAYS[i] + LYFA(cData[eYear], i);
	}
	lDay += cData[eDay];
	return lDay;
}

int TimeStamp::intTimeRep::WeekOfYear() const
{
	StartTrace(TimeStamp.WeekOfYear);
	long lDayOfYearZ = DayOfYear() - 1L;
	eWeekday aFirstOfYearDOW = (eWeekday)((((long)Weekday() + 7L) - (lDayOfYearZ % 7L)) % 7);
	Trace("firstofyearweekday:" << static_cast<long>(aFirstOfYearDOW));

	// week starts on monday
	long lDeltaDays = (((long)TimeStamp::eMonday + 7L) - static_cast<long>(aFirstOfYearDOW)) % 7;
	long lWeek = 1L, lDaysToCalcWeek = lDayOfYearZ;
	switch (aFirstOfYearDOW) {
		case TimeStamp::eMonday:
			break;
		case TimeStamp::eFriday:
		case TimeStamp::eSaturday:
		case TimeStamp::eSunday:
			--lWeek;
			/* fall through */
		case TimeStamp::eTuesday:
		case TimeStamp::eWednesday:
		case TimeStamp::eThursday:
			lDaysToCalcWeek += (7L - lDeltaDays);
			break;
		default:
			break;
	};
	Trace("Delta days to monday:" << lDeltaDays);
	Trace("day of year number:" << lDayOfYearZ + 1L);
	Trace("days for week calculation:" << lDaysToCalcWeek);

	lWeek += (lDaysToCalcWeek / 7);
	Trace("unadjusted week number:" << lWeek);
	if ( lWeek > 52 ) {
		eWeekday aEndOfYearDOW = (IsLeap(cData[eYear]) ? (eWeekday)((static_cast<long>(aFirstOfYearDOW) + 1L) % 7) : aFirstOfYearDOW);
		Trace("endofyearweekday:" << static_cast<long>(aEndOfYearDOW));
		switch (aEndOfYearDOW) {
			case TimeStamp::eMonday:
			case TimeStamp::eTuesday:
			case TimeStamp::eWednesday:
				lWeek = 1L;
				Trace("day/week already in new year!");
				break;
			default:
				break;
		};
	}
	Trace("adjusted week number:" << lWeek);
	return lWeek;
}

const char *TimeStamp::MonthName() const
{
	StartTrace(TimeStamp.MonthName);
	return Monthes[(int)fTimeStruct.Month()];
}

const char *TimeStamp::DayName() const
{
	StartTrace(TimeStamp.DayName);
	return Days[fTimeStruct.Weekday()];
}
