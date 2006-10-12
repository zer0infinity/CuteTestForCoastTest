/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DateRenderer_H
#define _DateRenderer_H

#include "config_renderer.h"
#include "Renderer.h"

//---- DateRenderer ----------------------------------------------------------------
//! <B>The DateRenderer is used to insert a date and time into the output stream</B>
/*!
<B>Configuration:</B><PRE>
{
	/Format 	Rendererspec		optional, default "\%c", format string a la strftime
	/Date		[Rendererspec|long]	optional, default 0, date/time in seconds
	/Offset  	Rendererspec		optional, default 0, offset in days added to the otherwise used time
	/DeltaGMT	long				optional, default 0 [0|1], if set to 1, the GMT difference [+|-][0-9] is added to the output
}
or just
{
	"\%c"		Rendererspec	optional, default "\%c", format string a la strftime
	0			long			optional, default 0, date/time in seconds
	0			long			optional, default 0, offset in days added to the otherwise used time
}
</PRE>
- Using the tag /Date a time/date may be explicitely specified:
The current time/date is always used, if no /Date entry was explicitly defined.
- Using the tag /Format a 'strftime'-type format string may be specified to
format the output.
- Using the tag /Offset an offset (in days) may be added to time/date.
Notice: When using a simple array for the parameters, the 1st entry must always
by the format string, the 2nd entry the date (in seconds) and the 3rd entry the
offset (in days). Trailing parameters may be ommited!
Localization of time values (GMT-offset) is implemented by calling localtime().
The formatting of the date depends on the strftime C library function and defaults
to '%C' (= 19 for the current century and 20 for the next).
\attention The buffer for constructing the time string is limited to 200 bytes!
\note Unix dates will break somewhen around year 2036
*/
class EXPORTDECL_RENDERER DateRenderer : public Renderer
{
public:
	DateRenderer(const char *name);

	void RenderAll(ostream &reply, Context &c, const ROAnything &data);
	const size_t gcMaxDateArraySize;
};

#endif		//not defined _DateRenderer_H
