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
#include "SysLog.h"
#include "TimeStamp.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- ComputDateRenderer ----------------------------------------------------------------
RegisterRenderer(ComputeDateRenderer);

//---- ComputeDateRenderer ----------------------------------------------------------------
ComputeDateRenderer::ComputeDateRenderer(const char *name)
	: Renderer(name)
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

ComputeDateRenderer::~ComputeDateRenderer()
{
}

long ComputeDateRenderer::GetMonthIndex( String month )
{
	StartTrace(ComputeDateRenderer.GetMonthIndex);
	long idx = 0;
	Trace( "month = [" << month << "]" );
	if ( fMonthTable.IsDefined(month) ) {
		idx =  fMonthTable[month].AsLong(0);
	}
	Trace( "index = [" << idx << "]" );
	return idx;
}

void ComputeDateRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(ComputeDateRenderer.RenderAll);
	TraceAny(config, "config");

	String strFromDate, strInputFormat;
	long lDayfromidx = -1L, lMonthfromidx = -1L, lMonthAbbrfromidx = -1L, lYearfromidx = -1L, lHourfromidx = -1L, lHour12fromidx = -1L, lAmPmfromidx = -1L, lMinutefromidx = -1L, lSecondfromidx = -1L;

	if (config.IsDefined("FromDate")) { // first mandantory datetime
		strFromDate = Renderer::RenderToString( ctx, config["FromDate"] );
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

		char iCent = 19, iYear = 70, iMonth = 1, iDay = 1, iHour = 0, iMin = 0, iSec = 0;
		if ( lDayfromidx >= 0 ) {
			Trace( "Daysubstring: <" << strFromDate.SubString(lDayfromidx, 2) << ">" );
			iDay = (char)strFromDate.SubString(lDayfromidx, 2).AsLong(0L);
		}
		if ( lMonthAbbrfromidx >= 0 ) {
			Trace( "Monthabbrsubstring: <" << strFromDate.SubString(lMonthAbbrfromidx, 3) << ">" );
			iMonth = (char)GetMonthIndex(strFromDate.SubString(lMonthAbbrfromidx, 3));
		} else {
			Trace( "Monthsubstring: <" << strFromDate.SubString(lMonthfromidx, 2) << ">" );
			iMonth = (char)strFromDate.SubString(lMonthfromidx, 2).AsLong(0L);
		}
		if ( lYearfromidx >= 0 ) {
			Trace( "Yearsubstring: <" << strFromDate.SubString(lYearfromidx, 4) << ">" );
			iYear = (char)(strFromDate.SubString(lYearfromidx, 4).AsLong(0L) % 100);
			iCent = (char)(strFromDate.SubString(lYearfromidx, 4).AsLong(0L) / 100);
		}
		if ( lHourfromidx >= 0 ) {
			Trace( "Hoursubstring: <" << strFromDate.SubString(lHourfromidx, 2) << ">" );
			iHour = (char)strFromDate.SubString(lHourfromidx, 2).AsLong(0L);
		}
		if ( lHour12fromidx >= 0 && lAmPmfromidx >= 0) {
			int hour12 = (int)strFromDate.SubString(lHour12fromidx, 2).AsLong(0L) ;
			String amPm = strFromDate.SubString(lAmPmfromidx, 1) ;
			Trace( "Hour12substring: <" << (long)hour12 << ">" );
			Trace( "AmPmsubstring: <" << amPm << ">" );
			if ( amPm == "p" || amPm == "P" ) {
				if ( hour12 == 12 ) {
					iHour = hour12 ;
				} else {
					iHour = hour12 + 12 ;
				}
			} else {
				if ( hour12 == 12 ) {
					iHour = 0 ;
				} else {
					iHour = hour12 ;
				}
			}
		}
		if ( lMinutefromidx >= 0 ) {
			Trace( "Minutesubstring: <" << strFromDate.SubString(lMinutefromidx, 2) << ">" );
			iMin = (int)strFromDate.SubString(lMinutefromidx, 2).AsLong(0L);
		}
		if ( lSecondfromidx >= 0 ) {
			Trace( "Secondsubstring: <" << strFromDate.SubString(lSecondfromidx, 2) << ">" );
			iSec = (int)strFromDate.SubString(lSecondfromidx, 2).AsLong(0L);
		}
		// output is seconds since 00:00:00 UTC, January 1, 1970
		reply << TimeStamp(iCent, iYear, iMonth, iDay, iHour, iMin, iSec).AsLong();
	} else {
		SysLog::Warning("ComputeDateRenderer::RenderAll: mandatory 'FromDate' slot is missing in configuration!");
	}
}
