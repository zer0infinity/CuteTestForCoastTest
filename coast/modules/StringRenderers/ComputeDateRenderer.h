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
#include "Renderer.h"

//---- ComputeDateRenderer ----------------------------------------------------------
//: comment Renderer
//	Structure of config:
//<PRE>	{
//		/DateFrom					Formatted Date/Timestring (valid from  Jan 1 1970 00:00 till Jan 19 2038 03:14
//		/InputFormat				Rendererspec, default: dd.mm.YYYY (00:00:00)
//									* dd 	for day of month as decimal number
//									* mm 	for month as decimal number
//									*    or
//									* bbb 	for month as English abbreviated month name
//									* YYYY 	for year with century as decimal number
//									* HH for hour (24-hour clock) as decimal number
//									*    or
//									* II for hour (12-hour clock) and pp for 'am' or 'pm'
//									* MM for minute as decimal number
//									* SS for second as decimal number
//									* -------------yy still to be implemented
//									returns this time in seconds (can be used as input for DateRenderer)
//	}</PRE>
//	Example 1:
//<PRE>	{
//		/DateFrom					"31.12.2002"
//	}</PRE>
//	Example 2:
//<PRE>	{
//		/DateFrom					"Feb 01 2002"
//		/InputFormat				"bbb dd YYYY"
//	}</PRE>
//<PRE>	{
//		/DateFrom					"Feb 01 2002 12:05:02"
//		/InputFormat				"bbb dd YYYY HH:MM:SS"
//	}</PRE>
// The follow features are missing :
// Just the operators "+", "-" and Offset
// Localization of time values (GMT-offset) is not implemented.

// NOTE: Unix dates will break somewhen around year 2036

class ComputeDateRenderer : public Renderer
{
public:
	//--- constructors
	ComputeDateRenderer(const char *name);
	~ComputeDateRenderer();

	//--- public api
	//:single line description of newmethod
	// further explanation of the purpose of the method
	// this may contain <B>HTML-Tags</B>
	// ...
	//!param: aParam - explanation of aParam (important : paramname - explanation )
	//!retv: explanation of return value
	//!prec: explanation of precondition for the method call
	//!postc: explanation of postcondition for the method call
	void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
protected:
	long GetMonthIndex( String month );
private:
	Anything fMonthTable;
};

#endif
