/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DateRenderer.h"
#include "SystemBase.h"
#include "Tracer.h"

using namespace Coast;

#include <cstdlib>
#include <time.h>

//---- DateRenderer ----------------------------------------------------------------
RegisterRenderer(DateRenderer);

DateRenderer::DateRenderer(const char *name)
	: Renderer(name)
	, gcMaxDateArraySize(200)
{
}

void DateRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(DateRenderer.Render);
	TraceAny(config, "config");

	long configSize = 0;
	if ( config.GetType() == AnyArrayType ) {
		configSize = config.GetSize();
	}

	//--- set the output format of the date (used by strftime)
	String formatStr;
	ROAnything roaFormat;
	if ( config.LookupPath(roaFormat, "Format") || ( ( configSize > 0 ) && (config.SlotName(0) == 0) && config.LookupPath(roaFormat, ":0") ) ) {
		formatStr = Renderer::RenderToString( ctx, roaFormat );
	} else {
		// set default format as a fallback
#if defined(WIN32)
		formatStr = "%c";
#else
		formatStr = "%C";
#endif
	}

	//--- set the date to be output
	time_t now;
	struct tm *tt;

	ROAnything roaCfg;
	if ( config.LookupPath(roaCfg, "Date") || ( ( configSize > 1 ) && (config.SlotName(1) == 0) && config.LookupPath(roaCfg, ":1") ) ) {
		now = Renderer::RenderToString(ctx, roaCfg).AsLong(0);
	} else {
		// use current time
		time(&now);
	}

	// set the offset to be used
	long offset = 0;
	ROAnything roaOffset;
	if ( config.LookupPath(roaOffset, "Offset") || ( ( configSize > 2 ) && (config.SlotName(1) == 0) && config.LookupPath(roaOffset, ":2") ) ) {
		String strOffset = Renderer::RenderToString( ctx, roaOffset );
		offset = strOffset.AsLong(0);
		// optional trailing 'd' stands for day offsets instead of seconds
		if ( strOffset.Length() && strOffset[strOffset.Length()-1] == 'd' ) {
			offset *= 86400;
		}
		now += offset;
	}

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
		if ( config["UseLocalTime"].AsBool(true) ) {
			tt = System::LocalTime(&now, &res);
		} else {
			tt = System::GmTime(&now, &res);
		}
		strftime(date, gcMaxDateArraySize, (const char *)formatStr, tt);
	}
	// PS: maybe date renderer should be made more independend of strftime?

	Trace("Result: <" << date << ">");
	reply << (date);
	ROAnything delta;
	if ( !gmt && config["DeltaGMT"].AsBool(false) ) {
		// timezone defines difference of local time to utc in seconds
		long lTimeDiffHours = timezone / (60 * 60);
		String strOut(" ");
		if ( lTimeDiffHours < 0L ) {
			strOut << "-";
		}
		if ( lTimeDiffHours < 10L ) {
			strOut << "0";
		}
		strOut << abs(lTimeDiffHours) << "00";
		Trace("timezone offset:" << strOut);
		reply << strOut;
	} else if ( config["DeltaGMTSeconds"].AsBool(false) ) {
		Trace("timezone offset[s]:" << timezone);
		reply << timezone;
	}
}
