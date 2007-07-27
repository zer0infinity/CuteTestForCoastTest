/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TimeStamp.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "System.h"
#include "SysLog.h"

//--- c-modules used -----------------------------------------------------------
#include <ctype.h>

const long TimeStamp::MIN = 60L;
const long TimeStamp::HOUR = TimeStamp::MIN * 60L;
const long TimeStamp::DAY = 24 * TimeStamp::HOUR;
const long TimeStamp::YEAR = 365 * TimeStamp::DAY;

static const long MDAYS [] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const int WeekDayCenturyCorrect[] = { 4, 2, 0, 6 };
static const int WeekDayMonthCorrectNoLeap[] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };
static const int WeekDayMonthCorrectLeap[] = { 6, 2, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };
static const char *Monthes[] = { "---", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static const char *Days[] = { "Sun", "Mon", "Tue", "Wed", "Tue", "Fri", "Sat" };

//---- TimeStamp ----------------------------------------------------------------
TimeStamp::TimeStamp(Allocator *pAlloc)
	: fTime(0)
	, fRep(15, pAlloc)
	, fTimeSet(false)
{
	StartTrace1(TimeStamp.TimeStamp, "empty");
	SetTimeT(time(0));
}

TimeStamp::TimeStamp(const TimeStamp &aTimeStamp)
{
	StartTrace1(TimeStamp.TimeStamp, "copy [" << aTimeStamp.fRep << "]");
	operator=(aTimeStamp);
}

//! With given UTC
TimeStamp::TimeStamp(TSIntNumberType utc, Allocator *pAlloc)
	: fTime(0)
	, fRep(15, pAlloc)
	, fTimeSet(false)
{
	StartTrace1(TimeStamp.TimeStamp, "time_t:" << utc);
	SetTimeT(utc);
}

TimeStamp::TimeStamp(const String &externalTimeRep, Allocator *pAlloc)
	: fTime(0)
	, fRep(15, pAlloc)
	, fTimeSet(false)
{
	StartTrace1(TimeStamp.TimeStamp, "String [" << externalTimeRep << "]");
	IntDoInit(externalTimeRep);
}

TimeStamp::TimeStamp(char iCent, char iYear, char iMonth, char iDay, char iHour, char iMin, char iSec, Allocator *pAlloc)
	: fTime(0)
	, fRep(15, pAlloc)
	, fTimeSet(false)
{
	StartTrace1(TimeStamp.TimeStamp, "components [" << iCent << ':' << iYear << ':' << iMonth << ':' << iDay << ':' << iHour << ':' << iMin << ':' << iSec << "]");
	IntSet(TimeStamp::intTimeRep(iCent, iYear, iMonth, iDay, iHour, iMin, iSec));
}

bool TimeStamp::IntSet(TimeStamp::intTimeRep aRep)
{
	StartTrace1(TimeStamp.IntSet, "intTimeRep [" << aRep.AsString() << "]");
	if ( !aRep.IsValidDate() ) {
		SYSWARNING("invalid input format [" << aRep.AsString() << "]");
		return false;
	} else {
		fRep.Trim(0);
		fTimeStruct = aRep;
		fTime = fTimeStruct.AsTimeT();
		Trace("new time:" << fTime);
		fTimeSet = true;
	}
	return true;
}

TimeStamp &TimeStamp::Set(const String &externalTimeRep)
{
	StartTrace(TimeStamp.Set);
	IntDoInit(externalTimeRep);
	return *this;
}

String TimeStamp::RemoveNonNumericalChars(const String &externalTimeRep)
{
	String ret;
	long lExt = externalTimeRep.Length();
	for (long i = 0; i < lExt; ++i) {
		const char c = externalTimeRep.At(i);
		if ( c && isdigit(c) ) {
			ret.Append(c);
			fTimeStruct.AddCharacter(c);
		}
	}
	return ret;
}

bool TimeStamp::IntDoInit(const String &externalTimeRep)
{
	StartTrace(TimeStamp.IntDoInit);
	fTimeSet = false;
	fTime = 0;
	fTimeStruct.Reset();
	fRep = RemoveNonNumericalChars(externalTimeRep);
	fRep.Trim(14L);
	String pattern("19700101000000");
	// fill missing chars from pattern
	long lRepLen = fRep.Length(), lMax = pattern.Length();
	while ( lRepLen < lMax ) {
		fRep.Append((char)pattern[lRepLen]);
		fTimeStruct.AddCharacter((char)pattern[lRepLen]);
		++lRepLen;
	}
	Trace("fRep: " << fRep);
	if ( !fTimeStruct.IsValidDate() ) {
		SYSWARNING("invalid input format [" << externalTimeRep << "]");
		return false;
	} else {
		Trace("valid parsed date " << fTimeStruct.AsString());
		fTime = fTimeStruct.AsTimeT();
		Trace("new time:" << fTime);
		fTimeSet = true;
	}
	return true;
}

bool TimeStamp::SetTimeT(TSIntNumberType lTime)
{
	StartTrace(TimeStamp.SetTimeT);
	bool bRet = false;
	fTimeSet = false;
	fRep.Trim(0);
	fTime = 0;
	if ( lTime >= 0 ) {
		if ( ( bRet = fTimeStruct.InitFromTimeT(lTime) ) ) {
			fTimeSet = true;
			fTime = lTime;
		}
	}
	return bRet;
}

long TimeStamp::AsLong() const
{
	StartTrace(TimeStamp.AsLong);
	return fTime;
}

bool TimeStamp::IsValid() const
{
	StartTrace(TimeStamp.IsValid);
	return fTimeSet;
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
	String strDate;
	strDate = fRep;
	if ( !strDate.Length() ) {
		strDate = IntTimeTAsString();
	}
	return strDate;
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
	String result("19691231235959");
	if ( fTime >= 0 && fTimeSet ) {
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
	fTime = aStamp.fTime;
	fRep = aStamp.fRep;
	fTimeSet = aStamp.fTimeSet;
	fTimeStruct = aStamp.fTimeStruct;
	return *this;
}

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

bool TimeStamp::intTimeRep::IsValidDate() const
{
	StartTrace(TimeStamp.IsValidDate);
	long year = (cCent * 100) + cYear;
	bool bRet = true;
	// sanity check, set fTime to -1 if errornous
	if (year < 1970L || year > 2037L ||
		cMonth < 1 || cMonth > 12 ||
		cDay < 1 || cDay > 31 ||
		cHour < 0 || cHour > 23 ||
		cMin < 0 || cMin > 59 ||
		cSec < 0 || cSec > 59 ||
		( 2 == cMonth && cDay > ( TimeStamp::IsLeap(year) ? 29 : 28)) ||
		( (4 == cMonth || 6 == cMonth || 9 == cMonth || 11 == cMonth) && cDay > 30) ) {
		bRet = false;
	}
	Trace("Date is " << (bRet ? "" : "not ") << "OK " << (bRet ? "" : (const char *)TraceIntValues()));
	return bRet;
}

String TimeStamp::intTimeRep::TraceIntValues() const
{
	String result;
	result.Append("Century:").Append((long)cCent).Append(" Year:").Append((long)cYear).Append(" Month:").Append((long)cMonth).Append(" Day:").Append((long)cDay).Append(" Hour:").Append((long)cHour).Append(" Min:").Append((long)cMin).Append(" Sec:").Append((long)cSec).Append(" Pos:").Append(fStructPos);
	return result;
}

// zero fill number
static char ChrNumbers[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
#define ZFNUM(result, num) if ( num <= 9 ) result.Append('0').Append( ChrNumbers[(int)num] ); else result.Append( ChrNumbers[(num/10)] ).Append( ChrNumbers[(int)num%10] );

String TimeStamp::intTimeRep::AsString() const
{
	StartTrace1(TimeStamp.AsString, "intTimeRep");
	Trace("internal values [" << TraceIntValues() << "]");
	// result has length 14, optimize buffer to hold 14 characters plus 0
	String result(15);
	// assemble string
	long year = (cCent * 100) + cYear;
	result.Append(year);
	ZFNUM(result, cMonth);
	ZFNUM(result, cDay);
	ZFNUM(result, cHour);
	ZFNUM(result, cMin);
	ZFNUM(result, cSec);
	Trace("result:" << result);
	return result;
}

TimeStamp::TSIntNumberType TimeStamp::intTimeRep::AsTimeT() const
{
	StartTrace(TimeStamp.AsTimeT);
	TSIntNumberType lTime = 0;
	long year = (cCent * 100) + cYear;
	// now calc the seconds
	lTime = (year - 1970) * TimeStamp::YEAR;
	Trace("number of leap years to account for:" << ((year - 1) / 4 - 1969 / 4));
	lTime += (((year - 1) / 4 - 1969 / 4)) * TimeStamp::DAY;
	// count leap years before current year, since 1970
	for (int i = 0; i < cMonth - 1; ++i) {
		lTime += MDAYS[i] * TimeStamp::DAY;
	}
	// leap-year adjustment
	if ( cMonth > 2 && TimeStamp::IsLeap(year) ) {
		Trace("adjusting current leap year");
		lTime += TimeStamp::DAY;
	}
	Trace("adding days:" << cDay - 1);
	lTime += (cDay - 1) * TimeStamp::DAY;
	lTime += cHour * TimeStamp::HOUR + cMin * TimeStamp::MIN + cSec;
	Trace("new time:" << lTime);
	return lTime;
}

bool TimeStamp::intTimeRep::InitFromTimeT(TSIntNumberType lTime)
{
	StartTrace(TimeStamp.InitFromTimeT);
	// normal case, do a brain dead standalone conversion, which should be fast!
	long days = lTime / TimeStamp::DAY;
	long rem = lTime % TimeStamp::DAY;
	cHour = (char)(rem / TimeStamp::HOUR);
	rem %= TimeStamp::HOUR;
	cMin = (char)(rem / 60);
	cSec = rem % 60;
	long y = 1970;
	long lYearDays = 0L;
	// count correct year, very simply
	while ( days >= ( lYearDays = (TimeStamp::IsLeap(y) ? 366 : 365) ) ) {
		// could be optimized but runs max about 80 times...
		days -= lYearDays;
		++y;
	}
	cCent = y / 100;
	cYear = y % 100;
	// calculate month
	int i = 0;
	// Leap Year February Adjust
#define LYFA(theyear,mindex) ((1==mindex&&TimeStamp::IsLeap(theyear))?1:0)
	for (i = 0; i < 12; ++i) {
		if (days >= MDAYS[i] + LYFA(y, i)) {
			days -= MDAYS[i] + LYFA(y, i);
		} else {
			break;
		}
	}
	cMonth = i + 1;
	days += 1;
	cDay = (char)days;
	return IsValidDate();
}

TimeStamp::eWeekday TimeStamp::intTimeRep::Weekday() const
{
	StartTrace(TimeStamp.Weekday);
	int y = (cCent * 100) + cYear;
	Trace("isleap:" << (TimeStamp::IsLeap(y) ? "true" : "false"));
	int DoW = WeekDayCenturyCorrect[cCent-17];
	DoW += cYear;
	DoW += (cYear / 4);
	DoW += (TimeStamp::IsLeap(y) ? WeekDayMonthCorrectLeap[cMonth-1] : WeekDayMonthCorrectNoLeap[cMonth-1]);
	DoW += cDay;
	DoW = (DoW % 7);
	Trace("DoW:" << DoW);
	return (TimeStamp::eWeekday)DoW;
}

int TimeStamp::intTimeRep::DayOfYear() const
{
	StartTrace(TimeStamp.DayOfYear);
	int lDay = 0;
	for (int i = 0; i < cMonth - 1; ++i) {
		lDay += MDAYS[i] + LYFA(cYear, i);
	}
	lDay += cDay;
	return lDay;
}

int TimeStamp::intTimeRep::WeekOfYear() const
{
	StartTrace(TimeStamp.WeekOfYear);
	long lDayOfYearZ = DayOfYear() - 1L;
	eWeekday aFirstOfYearDOW = (eWeekday)((((long)Weekday() + 7L) - (lDayOfYearZ % 7L)) % 7);
	Trace("firstofyearweekday:" << (long)aFirstOfYearDOW);

	// week starts on monday
	long lDeltaDays = (((long)TimeStamp::eMonday + 7L) - (long)aFirstOfYearDOW) % 7;
	long lWeek = 1L, lDaysToCalcWeek = lDayOfYearZ;
	switch (aFirstOfYearDOW) {
		case TimeStamp::eMonday:
			break;
		case TimeStamp::eFriday:
		case TimeStamp::eSaturday:
		case TimeStamp::eSunday:
			--lWeek;
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
		eWeekday aEndOfYearDOW = (IsLeap(cYear) ? (eWeekday)(((long)aFirstOfYearDOW + 1L) % 7) : aFirstOfYearDOW);
		Trace("endofyearweekday:" << (long)aEndOfYearDOW);
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
	return Monthes[(int)fTimeStruct.cMonth];
}

const char *TimeStamp::DayName() const
{
	StartTrace(TimeStamp.DayName);
	return Days[fTimeStruct.Weekday()];
}
