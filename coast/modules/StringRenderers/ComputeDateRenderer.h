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

//---- ComputeDateRenderer ----------------------------------------------------------
//! <b>Converts the given Date/Time according to the specified format into a bunch of seconds (unix-time, seconds since 1. Jan. 1970 00:00)</b>
/*!
<b>Configuration:</b><pre>
{
	/FromDate					Formatted Date/Timestring (valid from  Jan 1 1970 00:00 till Jan 19 2038 03:14
	/InputFormat				Rendererspec, default: dd.mm.YYYY (00:00:00)
								* dd 	for day of month as decimal number
								* mm 	for month as decimal number
								*    or
								* bbb 	for month as English abbreviated month name
								* YYYY 	for year with century as decimal number
								* HH for hour (24-hour clock) as decimal number
								*    or
								* II for hour (12-hour clock) and pp for 'am' or 'pm'
								* MM for minute as decimal number
								* SS for second as decimal number
								* -------------yy still to be implemented
								returns this time in seconds (can be used as input for DateRenderer)
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

protected:
	//! Render the requested slotname
	/*! \param month english month abbreviation to be converted into a number
		\return month number, Jan == 1 */
	long GetMonthIndex( String month );

private:
	Anything fMonthTable;
};

#endif
