/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Dbg_H
#define _Dbg_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION

//---- Tracer --------------------------------------------------------------------------
#ifdef DEBUG
#include "ITOString.h"

class EXPORTDECL_FOUNDATION Anything;
class EXPORTDECL_FOUNDATION ROAnything;
class EXPORTDECL_FOUNDATION TracerHelper;

//!class and macros, that provide tracing support
//!This macros help keep the coding expense for inserting trace statements
//!into the source-code at a minimum.<P>
//!If the preprocessor flag DEBUG is not set they expand into nothing.
//!To keep the trace output at acceptable levels we introduced a config
//!file for debugging Dbg.any. Therein you can switch the trace on and off.
//!<P><br>
//!Trace statements are scoped. In every scope of a C++-Program you can
//!use a StartTrace or StartTrace1 macro. They take as a first parameter
//!a trigger e.g. <B>StartTrace(Server.Load)</B>. This results into output onto
//!cerr like
//!<P><I>"Server.Load ---- entered ---"</I><P><I>"Server.Load ---- left ---"</I><P>
//!on entering resp. leaving the scope. Within the scope you can use as many
//!Trace macros as you need. They are all set with the same switch.
//!To set the switch you have to insert the following lines into <B>Dbg.any</B><P><P>
//!<I>Server {<P>
//!	/Load		10<P>
//!}</I><P>
//!<h3>List of Trace Macros</h3>
//!<ul>
//!<b>At start of scope:</b>
//!<li>StartTrace(trigger)<br> e.g. StartTracer(Server.Load);
//!<li>StartTrace1(trigger, messages)<br>
//!e.g. StartTrace(Server.Load, "file:<" << filename << ">");<p>
//!<b>Within scope: (as many as useful)</b>
//!<li>Trace(message)<br>
//!Trace("some text:" << aVariable << " some additional text");
//!<li>TraceAny(anything, message)<br>
//!TraceAny(anAnything, "an explaining Text like tmpStore:");<p>
//!<b>To restrict lengthy output:</b>
//!<li>SubTrace(subtrigger, messages);<br>
//!Trace(filename, "filname:" << filename << " found");
//!<li>SubTraceAny(subtrigger, anything, message);<br>
//!Trace(tmpStore, tmp, "tmpStore at open:");
//!</ul>

class EXPORTDECL_FOUNDATION Tracer
{
public:
	Tracer(const char *trigger);
	Tracer(const char *trigger, const char *msg);

	~Tracer();

	void Use() const { }

	void WDDebug(const char *msg);
	void AnyWDDebug(const ROAnything &any, const char *msg);
	void SubWDDebug(const char *trigger, const char *msg);
	void SubAnyWDDebug(const char *trigger, const ROAnything &any, const char *msg);
	static void StatWDDebug(const char *trigger, const char *msg, Allocator *pAlloc);
	static void AnythingWDDebug(const char *trigger, const ROAnything &a, const char *msg, Allocator *pAlloc);
	static bool CheckWDDebug(const char *trigger, Allocator *pAlloc);

	static void Reset();
	static void Terminate();

protected:
	friend class DbgTest;
	static bool CheckTrigger(const char *trigger, Allocator *pAlloc);

	static bool DoCheckLevel(const char *trigger, const ROAnything &level, long levelSwitch, long levelAll, long enableAll, Allocator *pAlloc);
	static bool DoCheckTrigger(const char *trigger, const ROAnything &level, long levelSwitch, long levelAll, long enableAll, Allocator *pAlloc);
	static bool DoCheckSwitch(long switchValue, long levelSwitch, long levelAll);
	static bool CheckMainSwitch(long mainSwitch, long levelSwitch, long levelAll);

private:
	static void IntAnyWDDebug(const ROAnything &any, TracerHelper &hlp);

	const char *fTrigger;
	bool fTriggered;
	const char *fpMsg;
	Allocator *fpAlloc;

	static int fgLevel;
	static Anything fgWDDebugContext;
	static ROAnything fgROWDDebugContext;
	static long fgLowerBound;
	static long fgUpperBound;
	static long fgAlwaysOn;
	static bool fgDumpAnythings;
	static bool fgIsOk;
	static bool fgTerminated;
};

// constructor
#define StartTrace(trigger) \
	Tracer recart(_QUOTE_(trigger)); recart.Use()

#define StartTrace1(trigger, msg) \
	String gsMrotcurtsnoCrecart(Storage::Current()); \
	Tracer recart(_QUOTE_(trigger), gsMrotcurtsnoCrecart << msg); recart.Use()

// debug statements
#define Trace(msg)									\
{													\
	String gsMecart(Storage::Current());				\
	recart.WDDebug(gsMecart << msg);					\
}

#define TraceBuf(buf, sz)				\
{										\
	String gsMecart("\n\n<",-1, Storage::Current());			\
	gsMecart.Append((const void*)buf, sz).Append(">\n\n");	\
	recart.WDDebug(gsMecart);			\
}

#define TraceAny(any, msg)					\
{											\
	String gsMecart(Storage::Current());		\
	recart.AnyWDDebug(any, gsMecart << msg);	\
}

// subdebugs
#define SubTrace(subtrigger, msg)							\
{															\
	String gsMecart(Storage::Current());						\
	recart.SubWDDebug(_QUOTE_(subtrigger), gsMecart << msg);	\
}

#define SubTraceBuf(subtrigger, buf, sz)					\
{															\
	String gsMecart("\n\n<",-1, Storage::Current());			\
	gsMecart.Append((const void*)buf, sz).Append(">\n\n");	\
	recart.SubWDDebug(_QUOTE_(subtrigger), gsMecart);			\
}

#define SubTraceAny(subtrigger, any, msg)							\
{																	\
	String gsMecart(Storage::Current());								\
	recart.SubAnyWDDebug(_QUOTE_(subtrigger), any, gsMecart << msg);	\
}

// static traces
#define StatTrace(trigger, msg, allocator)						\
{															\
	String gsMecart(allocator);						\
	Tracer::StatWDDebug(_QUOTE_(trigger), gsMecart << msg, allocator);	\
}

#define StatTraceBuf(trigger, buf, sz, allocator)					\
{															\
	String gsMecart("\n\n<",-1, allocator);			\
	gsMecart.Append((const void*)buf, sz).Append(">\n\n");	\
	Tracer::StatWDDebug(_QUOTE_(trigger), gsMecart, allocator);	\
}

#define StatTraceAny(trigger, any, msg, allocator)						\
{															\
	String gsMecart(allocator);						\
	Tracer::AnythingWDDebug(_QUOTE_(trigger), any, gsMecart << msg, allocator);	\
	Tracer::StatWDDebug(_QUOTE_(trigger), gsMecart << msg, allocator);	\
}

// helper to check if we are triggered
#define TraceTriggered(trigger, allocator)							\
	Tracer::CheckWDDebug(_QUOTE_(trigger), allocator)

#define ResetTracer()	Tracer::Reset()
#define TerminateTracer()	Tracer::Terminate()
#define HTMLTraceStores(reply, context)	context.HTMLDebugStores(reply)

#else
//--- optimized versions of the debug macros they expand into nothing
// constructor
#define StartTrace(trigger)
#define StartTrace1(trigger, msg)
// debug statements
#define TraceBuf(buf, sz)	;
#define Trace(msg)	;
#define TraceAny(any, msg)	;
// subdebugs
#define SubTrace(subtrigger, msg)
#define SubTraceAny(subtrigger, any, msg)
#define SubTraceBuf(subtrigger, buf, sz)
// static trace facility
#define StatTrace(trigger, msg, allocator)
#define StatTraceBuf(trigger, buf, sz, allocator)
#define StatTraceAny(trigger, any, msg, allocator)
// helper to check if we are triggered
#define TraceTriggered(trigger, allocator)		false

#define ResetTracer()
#define TerminateTracer()
#define HTMLTraceStores(reply, context)
#endif

#endif		//not defined _Dbg_H
