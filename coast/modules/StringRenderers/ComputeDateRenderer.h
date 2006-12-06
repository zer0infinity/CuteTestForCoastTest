/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ComputeDateRenderer_H
#define _ComputeDateRenderer_H

//---- baseclass include -------------------------------------------------
#include "config_StringRenderers.h"
#include "Renderer.h"

class TimeStamp;
//---- ComputeDateRenderer ----------------------------------------------------------
//! <b>Converts the given Date/Time according to the specified format into a bunch of seconds (unix-time, seconds since 1. Jan. 1970 00:00)</b>
/*!
<b>Configuration:</b><pre>
{
	/FromDate		Rendererspec		mandatory, Formatted Date/Timestring (valid from  Jan 1 1970 00:00 till Jan 19 2038 03:14
	/InputFormat	Rendererspec		optional, default: dd.mm.YYYY (00:00:00)
											* dd 	for day of month as decimal number
											* mm 	for month as decimal number
											*    or
											* bbb 	for month as English abbreviated month name
											* YY 	for year without century as decimal number, assume values above or equal 70 to be in century 19, others in century 20
											* YYYY 	for year with century as decimal number
											* HH for hour (24-hour clock) as decimal number
											*    or
											* MM for minute as decimal number
											* SS for second as decimal number
											* pp for hour (12-hour clock) 'am' or 'pm'
											* -------------yy still to be implemented
											returns this time in seconds (can be used as input for DateRenderer)
	/Offset  		Rendererspec		optional, default 0, offset in seconds or days (if a trailing 'd' was given) added to the time
}</pre>
Example 1:
<pre>
{
	/FromDate					"31.12.2002"
}</pre>
Example 2:
<pre>
{
	/FromDate					"Feb 01 2002"
	/InputFormat				"bbb dd YYYY"
}</pre>
Example 3:
<pre>
{
	/FromDate					"Feb 01 2002 12:05:02"
	/InputFormat				"bbb dd YYYY HH:MM:SS"
}</pre>
Example 4:
<pre>
{
	/FromDate					"Feb 01 2002 12:05:02AM"
	/InputFormat				"bbb dd YYYY HH:MM:SSpp"
}</pre>
The follow features are missing :
Just the operators "+", "-" and Offset
Localization of time values (GMT-offset) is not implemented.
*/
class EXPORTDECL_STRINGRENDERERS ComputeDateRenderer : public Renderer
{
public:
	ComputeDateRenderer(const char *name);
	~ComputeDateRenderer();

	//! Render the requested slotname
	/*! \param reply stream to generate output on
		\param c Context used for output generation
		\param config configuration which drives the output generation */
	void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);

	//! convert the given from date string into a TimeStamp using the specified scan format
	/*! \param strFromDate date string to convert
		\param strInputFormat string defining the format of the strFromDate string, syntax as described
		\return TimeStamp of the given FromDate if it could be converted, 19700101000000 otherwise */
	TimeStamp ConvertToTimeStamp(const String &strFromDate, const String &strInputFormat);

	//! return a month index given the english month abbreviation
	/*! \param month english month abbreviation to be converted into a number
		\return month number, Jan == 1 */
	long GetMonthIndex( String month );

private:
	//! return a number parsed within the string using the index given
	/*! \param strFromDate date string to get the number from
		\param lIdx index from where to start parsing within the given string, this index is zero based
		\param lMaxIdx maximal position within string, to be able to limit number of digits, eg a two digit number will need lMaxIdx=lIdx+2
		\return The converted value */
	long IntGetValue(const String &strFromDate, long &lIdx, long lMaxIdx);
};

#endif
