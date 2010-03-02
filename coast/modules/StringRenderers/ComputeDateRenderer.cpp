/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include -------------------------------------------------------
#include "ComputeDateRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "TimeStamp.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

static const char *pcMonthes[] = { "---", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

//---- ComputDateRenderer ----------------------------------------------------------------
RegisterRenderer(ComputeDateRenderer);

//---- ComputeDateRenderer ----------------------------------------------------------------
ComputeDateRenderer::ComputeDateRenderer(const char *name)
	: Renderer(name)
{
}

ComputeDateRenderer::~ComputeDateRenderer()
{
}

long ComputeDateRenderer::GetMonthIndex( String month )
{
	StartTrace(ComputeDateRenderer.GetMonthIndex);
	long idx = (sizeof(pcMonthes) / sizeof(char *));
	while ( --idx > 0 && !month.IsEqual(pcMonthes[idx]) ) {
		Trace("idx:" << idx << " month[" << pcMonthes[idx] << "]");
	}
	Trace( "index = [" << idx << "]" );
	return idx;
}

void ComputeDateRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(ComputeDateRenderer.RenderAll);
	TraceAny(config, "config");

	if (config.IsDefined("FromDate")) { // first mandantory datetime
		String strFromDate, strInputFormat;
		strFromDate = Renderer::RenderToString( ctx, config["FromDate"] );
		if (config.IsDefined("InputFormat")) { // first mandantory datetime
			Renderer::RenderOnString( strInputFormat, ctx, config["InputFormat"]);
		} else {
			strInputFormat = "dd.mm.YYYY"; // time is 00:00:00
		}

		long offset = 0;
		ROAnything roaOffset;
		if ( config.LookupPath(roaOffset, "Offset") ) {
			String strOffset = Renderer::RenderToString( ctx, roaOffset );
			offset = strOffset.AsLong(0);
			// optional trailing 'd' stands for day offsets instead of seconds
			if ( strOffset.Length() && strOffset[strOffset.Length()-1] == 'd' ) {
				offset *= 86400;
			}
		}

		Trace("Fromdate:" << strFromDate << ", InputFormat:" << strInputFormat << ", Offset[s]:" << offset);

		// output is seconds since 00:00:00 UTC, January 1, 1970
		reply << ( ConvertToTimeStamp(strFromDate, strInputFormat).AsLong() + offset );
	} else {
		SystemLog::Warning("ComputeDateRenderer::RenderAll: mandatory 'FromDate' slot is missing in configuration!");
	}
}

long ComputeDateRenderer::IntGetValue(const String &strFromDate, long &lIdx, long lMaxIdx)
{
	StartTrace(ComputeDateRenderer.IntGetValue);
	long lVal = 0;
	char cV = strFromDate[lIdx];
	Trace("character to convert '" << cV << "'");
	while ( lIdx < lMaxIdx && cV >= '0' && cV <= '9' ) {
		lVal *= 10;
		lVal += (cV - '0');
		Trace("new value: " << lVal);
		++lIdx;
		cV = strFromDate[lIdx];
		Trace("character to convert '" << cV << "'");
	}
	Trace("returning value:" << lVal);
	return lVal;
}

TimeStamp ComputeDateRenderer::ConvertToTimeStamp(const String &strFromDate, const String &strInputFormat)
{
	StartTrace(ComputeDateRenderer.ConvertToTimeStamp);
	Trace("Fromdate:" << strFromDate << ", InputFormat:" <<  strInputFormat );
	long lDtIdx = 0;
	char iCent = 19, iYear = 70, iMonth = 1, iDay = 1, iHour = 0, iMin = 0, iSec = 0;
	String strInFmt = strInputFormat;
	while ( strInFmt.Length() ) {
		Trace("current FmtString [" << strInFmt << "]");
		Trace("");
		if ( strInFmt.StartsWith("dd") ) {
			iDay = (char)IntGetValue(strFromDate, lDtIdx, lDtIdx + 2);
			Trace("day: " << (long)iDay);
			strInFmt.TrimFront(2);
		} else if ( strInFmt.StartsWith("mm") ) {
			iMonth = (char)IntGetValue(strFromDate, lDtIdx, lDtIdx + 2);
			Trace("month: " << (long)iMonth);
			strInFmt.TrimFront(2);
		} else if ( strInFmt.StartsWith("bbb") ) {
			iMonth = (char)GetMonthIndex(strFromDate.SubString(lDtIdx, 3));
			Trace("month: " << (long)iMonth);
			strInFmt.TrimFront(3);
			lDtIdx += 3;
		} else if ( strInFmt.StartsWith("YYYY") ) {
			long lYear = IntGetValue(strFromDate, lDtIdx, lDtIdx + 4);
			iYear = (char)(lYear % 100);
			iCent = (char)(lYear / 100);
			Trace("year: " << (long)iYear);
			Trace("cent: " << (long)iCent);
			strInFmt.TrimFront(4);
		} else if ( strInFmt.StartsWith("YY") ) {
			iYear = (char)IntGetValue(strFromDate, lDtIdx, lDtIdx + 2);
			iCent = 20;
			Trace("year: " << (long)iYear);
			if ( iYear >= 70 ) {
				iCent = 19;
			}
			Trace("cent: " << (long)iCent);
			strInFmt.TrimFront(2);
		} else if ( strInFmt.StartsWith("HH") ) {
			iHour = (char)IntGetValue(strFromDate, lDtIdx, lDtIdx + 2);
			Trace("hour: " << (long)iHour);
			strInFmt.TrimFront(2);
		} else if ( strInFmt.StartsWith("MM") ) {
			iMin = (char)IntGetValue(strFromDate, lDtIdx, lDtIdx + 2);
			Trace("min: " << (long)iMin);
			strInFmt.TrimFront(2);
		} else if ( strInFmt.StartsWith("SS") ) {
			iSec = (char)IntGetValue(strFromDate, lDtIdx, lDtIdx + 2);
			Trace("sec: " << (long)iSec);
			strInFmt.TrimFront(2);
		} else if ( strInFmt.StartsWith("pp") ) {
			String amPm = strFromDate.SubString(lDtIdx, 2);
			Trace( "AmPmsubstring: <" << amPm << ">" );
			if ( amPm.IsEqual("pm") || amPm.IsEqual("PM") ) {
				if ( iHour != 12 ) {
					iHour += 12;
				}
			} else {
				if ( iHour == 12 ) {
					iHour = 0 ;
				}
			}
			lDtIdx += 2;
			strInFmt.TrimFront(2);
		} else if ( strInFmt[0L] == 'X' ) {
			++lDtIdx;
			strInFmt.TrimFront(1);
		} else {
			// unrecognized (specifier) character, check if we have to skip it in the date string too
			while ( strFromDate[lDtIdx] == strInFmt[0L] ) {
				++lDtIdx;
			}
			strInFmt.TrimFront(1);
		}
	}
	// output is seconds since 00:00:00 UTC, January 1, 1970
	return TimeStamp(iCent, iYear, iMonth, iDay, iHour, iMin, iSec);
}
