/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "DateRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#include <time.h>

//---- DateRenderer ----------------------------------------------------------------
RegisterRenderer(DateRenderer);

const size_t gcMaxDateArraySize = 200;

DateRenderer::DateRenderer(const char *name) : Renderer(name)
{
}

void DateRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(DateRenderer.Render);
	TraceAny(config, "config");

	long configSize = 0;
	if (config.GetType() == Anything::eArray) {
		configSize = config.GetSize();
	}

	//--- set the output format of the date (used by strftime)
	const char *format = 0;

	String formatStr;
	ROAnything roaFormat;
	if (config.LookupPath(roaFormat, "Format")) {
		Renderer::RenderOnString( formatStr, ctx, roaFormat );
		format = formatStr;
	} else if (( configSize > 0 ) && (config.SlotName(0 == 0)) ) {
		// use anonymous format, but only if anonymous!
		format = config[0L].AsCharPtr(0);	// use first entry as format (if present)
	}
	if (format == 0)
#if defined(WIN32)
		format = "%c";						// set default format as a fallback
#else
		format = "%C";						// set default format as a fallback
#endif

	//--- set the date to be output
	time_t now;
	struct tm *tt;

	ROAnything d;
	if (config.LookupPath(d, "Date")) {
		if (d.GetType() == Anything::eLong ) {
			now = d.AsLong(0);				// check for 'Date' slot
		} else if (d.GetType() == Anything::eArray) { // it is a renderer
			String aDate;
			Renderer::RenderOnString(aDate, ctx, d);
			Anything anyDate = aDate;
			now = anyDate.AsLong(0);
		}

	} else if (( configSize > 1 ) && (config.SlotName(1) == 0)) {
		// use anonymous format, but only if anonymous!
		now = config[1L].AsLong(0);		// use 2nd entry as time (if present)
	} else {
		time(&now);						// use current time
	}

	// set the offset to be used (in days)
	int offset = 0;

	String offsetStr;
	ROAnything roaOffset;
	if (config.LookupPath(roaOffset, "Offset")) {
		Renderer::RenderOnString( offsetStr, ctx, roaOffset );
		offset = offsetStr.AsLong(0);
	} else if (( configSize > 2 ) && (config.SlotName(1) == 0)) {
		// use anonymous format, but only if anonymous!
		offset = config[2L].AsLong(0);	// use 3rd entry as offset
	}

	now = now + offset * 86400;			// (offset in days!)

	struct tm res;
	bool gmt(config["RFC2616Date"].AsBool(0));
	// NOTICE: array is limited to gcMaxDateArraySize chars
	char date[gcMaxDateArraySize];
	if ( gmt ) {
		// %Z would return local timezone, which is not what we want
		tt = System::GmTime(&now, &res);
		strftime(date, gcMaxDateArraySize, "%a, %d %b %Y %T GMT", tt);
	} else {
		// perform localization of time values
		tt = System::LocalTime(&now, &res);
		strftime(date, gcMaxDateArraySize, format, tt);
	}
	// PS: maybe date renderer should be made more independend of strftime?

	Trace("Result: <" << date << ">");
	reply << (date);
	ROAnything delta;
	if ( !gmt && config["DeltaGMT"].AsBool(0) ) {
		// timezone defines difference of local time to utc in seconds
		long lTimeDiffHours = timezone / (60 * 60);
		reply << " ";
		if ( lTimeDiffHours < 0L ) {
			reply << "-";
		}
		if ( lTimeDiffHours < 10L ) {
			reply << "0";
		}
		reply << abs(lTimeDiffHours) << "00";
	}
}
