/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-modules used -----------------------------------------------------------
#include <time.h>
// #include <ctime>
#include <iostream.h>

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "System.h"
#include "SysLog.h"
#include "Context.h"
#include "Dbg.h"

//--- interface include -------------------------------------------------------
#include "ComputeDateRenderer.h"

//---- ComputDateRenderer ----------------------------------------------------------------
RegisterRenderer(ComputeDateRenderer);

//---- ComputeDateRenderer ----------------------------------------------------------------
ComputeDateRenderer::ComputeDateRenderer(const char *name) : Renderer(name)
{

	fMonthTable["---"] = 0L;
	fMonthTable["Jan"] = 1L;
	fMonthTable["Feb"] = 2L;
	fMonthTable["Mar"] = 3L;
	fMonthTable["Apr"] = 4L;
	fMonthTable["May"] = 5L;
	fMonthTable["Jun"] = 6L;
	fMonthTable["Jul"] = 7L;
	fMonthTable["Aug"] = 8L;
	fMonthTable["Sep"] = 9L;
	fMonthTable["Oct"] = 10L;
	fMonthTable["Nov"] = 11L;
	fMonthTable["Dec"] = 12L;

}

ComputeDateRenderer::~ComputeDateRenderer() { }

long ComputeDateRenderer::GetMonthIndex( String month )
{
	StartTrace(ComputeDateRenderer.GetMonthIndex);

	Trace( "month = [" << month << "]" );
	if ( fMonthTable.IsDefined(month) ) {
		long idx =  fMonthTable[month].AsLong(0);
		Trace( "index = [" << idx << "]" );
		return idx;
	} else {
		Trace( "index = [0]" );
		return 0;
	}
}

void ComputeDateRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(ComputeDateRenderer.RenderAll);
	TraceAny(config, "config");

	time_t timefrom;
	String strFromDate, strInputFormat;
	long lDayfromidx = -1L, lMonthfromidx = -1L, lMonthAbbrfromidx = -1L, lYearfromidx = -1L, lHourfromidx = -1L, lHour12fromidx = -1L, lAmPmfromidx = -1L, lMinutefromidx = -1L, lSecondfromidx = -1L;

	if (config.IsDefined("FromDate")) { // first mandantory datetime
		Renderer::RenderOnString( strFromDate, ctx, config["FromDate"] );
		if (config.IsDefined("InputFormat")) { // first mandantory datetime
			Renderer::RenderOnString( strInputFormat, ctx, config["InputFormat"]);
		} else {
			strInputFormat = "dd.mm.YYYY"; // time is 00:00:00
		}
		Trace("Fromdate:" << strFromDate << ", InputFormat:" <<  strInputFormat );

		lDayfromidx = strInputFormat.Contains("dd");
		lMonthfromidx = strInputFormat.Contains("mm");
		lMonthAbbrfromidx = strInputFormat.Contains("bbb");
		lYearfromidx = strInputFormat.Contains("YYYY");
		lHourfromidx = strInputFormat.Contains("HH");
		lHour12fromidx = strInputFormat.Contains("II");
		lAmPmfromidx = strInputFormat.Contains("pp");
		lMinutefromidx = strInputFormat.Contains("MM");
		lSecondfromidx = strInputFormat.Contains("SS");

		Trace( "lDayfromidx: at index <" << lDayfromidx << ">" );
		Trace( "lMonthfromidx: at index <" << lMonthfromidx << ">" );
		Trace( "lMonthAbbrfromidx: at index <" << lMonthAbbrfromidx << ">" );
		Trace( "lYearfromidx: at index <" << lYearfromidx << ">" );
		Trace( "lHourfromidx: at index <" << lHourfromidx << ">" );
		Trace( "lHour12fromidx: at index <" << lHour12fromidx << ">" );
		Trace( "lAmPmfromidx: at index <" << lAmPmfromidx << ">" );
		Trace( "lMinutefromidx: at index <" << lMinutefromidx << ">" );
		Trace( "lSecondfromidx: at index <" << lSecondfromidx << ">" );

		struct tm date1;
		if ( lDayfromidx >= 0 ) {
			Trace( "Daysubstring: <" << strFromDate.SubString(lDayfromidx, 2) << ">" );
			date1.tm_mday = (int)strFromDate.SubString(lDayfromidx, 2).AsLong(0L);
		}
		if ( lMonthAbbrfromidx >= 0 ) {
			Trace( "Monthabbrsubstring: <" << strFromDate.SubString(lMonthAbbrfromidx, 3) << ">" );
			date1.tm_mon = GetMonthIndex(strFromDate.SubString(lMonthAbbrfromidx, 3)) - 1 ;
		} else {
			Trace( "Monthsubstring: <" << strFromDate.SubString(lMonthfromidx, 2) << ">" );
			date1.tm_mon = (int)strFromDate.SubString(lMonthfromidx, 2).AsLong(0L) - 1 ;
		}
		if ( lYearfromidx >= 0 ) {
			Trace( "Yearsubstring: <" << strFromDate.SubString(lYearfromidx, 4) << ">" );
			date1.tm_year = (int)strFromDate.SubString(lYearfromidx, 4).AsLong(0L) - 1900 ;
		}
		date1.tm_hour = 0;
		if ( lHourfromidx >= 0 ) {
			Trace( "Hoursubstring: <" << strFromDate.SubString(lHourfromidx, 2) << ">" );
			date1.tm_hour = (int)strFromDate.SubString(lHourfromidx, 2).AsLong(0L);
		}
		if ( lHour12fromidx >= 0 && lAmPmfromidx >= 0) {
			int hour12 = (int)strFromDate.SubString(lHour12fromidx, 2).AsLong(0L) ;
			String amPm = strFromDate.SubString(lAmPmfromidx, 1) ;
			Trace( "Hour12substring: <" << (long)hour12 << ">" );
			Trace( "AmPmsubstring: <" << amPm << ">" );
			if ( amPm == "p" || amPm == "P" ) {
				if ( hour12 == 12 ) {
					date1.tm_hour = hour12 ;
				} else {
					date1.tm_hour = hour12 + 12 ;
				}
			} else {
				if ( hour12 == 12 ) {
					date1.tm_hour = 0 ;
				} else {
					date1.tm_hour = hour12 ;
				}
			}
		}
		date1.tm_min = 0;
		if ( lMinutefromidx >= 0 ) {
			Trace( "Minutesubstring: <" << strFromDate.SubString(lMinutefromidx, 2) << ">" );
			date1.tm_min = (int)strFromDate.SubString(lMinutefromidx, 2).AsLong(0L);
		}
		date1.tm_sec = 0;
		if ( lSecondfromidx >= 0 ) {
			Trace( "Secondsubstring: <" << strFromDate.SubString(lSecondfromidx, 2) << ">" );
			date1.tm_sec = (int)strFromDate.SubString(lSecondfromidx, 2).AsLong(0L);
		}
		Trace("date1.tm_mday:" << (long)date1.tm_mday) ;
		Trace("date1.tm_mon:" << (long)date1.tm_mon );
		Trace("date1.tm_year:" << (long)date1.tm_year );
		Trace("date1.tm_hour:" << (long)date1.tm_hour );
		Trace("date1.tm_min:" << (long)date1.tm_min );
		Trace("date1.tm_sec:" << (long)(date1.tm_sec) );
		Trace("date1.tm_isdst:" << (long)(date1.tm_isdst) );

		timefrom = mktime( &date1 ); 		// convert struct tm to seconds

		Trace("date1.tm_mday:" << (long)date1.tm_mday) ;
		Trace("date1.tm_mon:" << (long)date1.tm_mon );
		Trace("date1.tm_year:" << (long)date1.tm_year );
		Trace("date1.tm_hour:" << (long)date1.tm_hour );
		Trace("date1.tm_min:" << (long)date1.tm_min );
		Trace("date1.tm_sec:" << (long)(date1.tm_sec) );
		Trace("date1.tm_isdst:" << (long)(date1.tm_isdst) );

		reply << (long)timefrom; 	// output is seconds since 00:00:00 UTC, January 1, 1970

	} else {
		SysLog::Warning("ComputeDateRenderer::RenderAll: mandatory 'FromDate' slot is missing in configuration!");
	}
}
