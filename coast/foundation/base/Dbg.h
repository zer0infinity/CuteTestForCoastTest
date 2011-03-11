/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Dbg_H
#define _Dbg_H

#include "ITOString.h"

//---- Tracer --------------------------------------------------------------------------
#ifdef COAST_TRACE

class Allocator;
class Anything;
class ROAnything;

//! Macros to simplify the task of printing out messages to the console together with full control of what to print.
/*! \file
The trace facility of Coast is very powerful due to its flexibility based on configuration in a file. To enable/disable
trace output, no recompilation of code is necessary as long as \em COAST_TRACE was defined at compile time.
Due to evaluation of the trace calls at runtime, expect a slight overhead. To globally disable tracing when executing a
program, set \em COAST_NO_TRACE environment variable prior to starting.

\par Preprocessor Flags
If the preprocessor flag \em COAST_TRACE is not set, the macros described here expand into nothing. To keep the trace output at acceptable levels we introduced a config
file for debugging, \b Dbg.any. Therein you can switch the trace on and off at a global level but also at method level if needed.

\par Trace mechanics
Trace statements are scoped unless the Static ones are used. In every scope of a C++-Program you can use a StartTrace() or StartTrace1() macro. They take as a first parameter
a trigger e.g. <B>StartTrace(Server.Load)</B>. This results into output onto cerr like
\code
Server.Load --- entering ---
	<more trace output of this method or enabled submethods>
Server.Load --- leaving ---
\endcode
on entering resp. leaving the scope. Within the scope you can place as many Trace() macros as you need. They are all enabled with the same switch.
To enable them, you have to insert the following lines into \b Dbg.any:
\code
{
	/LowerBound		10
	/UpperBound		20
	/DumpAnythings	1

	/Server {
		/MainSwitch	10
		/EnableAll	0
		/Load		10
	}
}
\endcode

\par Structure of \b Dbg.any
\code
{
	/LowerBound		10
	/UpperBound		20
	/DumpAnythings	1

	/Server {
		/MainSwitch	10
		/EnableAll	0
		/Load		10
	}
}
\endcode

\par List of Trace Macros
The following macros help keep the coding expense for inserting trace statements into the source-code at a minimum.
	- <b>At start of scope:</b>
		- StartTrace(trigger) \code StartTrace(Server.Load); \endcode
		- StartTrace1(trigger, messages) \code StartTrace(Server.Load, "file:<" << filename << ">"); \endcode
	- <b>Within scope: (as many as useful)</b>
		- Trace(message) \code Trace("some text:" << aVariable << " some additional text"); \endcode
		- TraceAny(anything, message) \code TraceAny(anAnything, "an explaining Text like tmpStore:"); \endcode
	- <b>To restrict lengthy output:</b>
		- SubTrace(subtrigger, messages) \code SubTrace(filename, "filname:" << filename << " found"); \endcode
		- SubTraceAny(subtrigger, anything, message) \code SubTraceAny(tmpStore, tmp, "tmpStore at open:"); \endcode
*/

//! class and macros, that provide tracing support
/*! Detailed description of how tracing works can be found in \b Dbg.h */
class Tracer
{
	Tracer(const Tracer &);
	Tracer &operator=(const Tracer &);
public:
	//! Contructor getting trigger only argument
	/*! \param trigger Will internally be used to do an Anything::LookupPath() search inside the \b Dbg.any file to check if trace output should be enabled or not */
	Tracer(const char *trigger);

	//! Contructor getting trigger and additional message as arguments
	/*! \param trigger Will internally be used to do an Anything::LookupPath() search inside the \b Dbg.any file to check if trace output should be enabled or not
		\param msg additional message to print out when constructing/destructing Tracer object */
	Tracer(const char *trigger, const char *msg);

	//! Destructor
	/*! if a message was specified when constructing the object, it will be printed out during destruction */
	~Tracer();

	void Use() const { }

	//! print out message \em msg
	/*! \param msg message to print out */
	void WDDebug(const char *msg);

	//! print out content of an Anything
	/*! \param any (RO)Anything to print out
		\param msg message to print out */
	void AnyWDDebug(const ROAnything &any, const char *msg);

	//! print out \em msg when additional \em trigger is activated
	/*! \param trigger additional sublevel within current trigger scope
		\param msg message to print out */
	void SubWDDebug(const char *trigger, const char *msg);

	//! print out \em any when additional \em trigger is activated
	/*! \param trigger additional sublevel within current trigger scope
		\param any (RO)Anything to print out
		\param msg message to print out */
	void SubAnyWDDebug(const char *trigger, const ROAnything &any, const char *msg);

	//! print out single \em msg when \em trigger is activated independent of other trigger scopes
	/*! \param trigger scope of trigger to check for
		\param msg message to print out
		\param pAlloc Allocator to use for allocating memory */
	static void StatWDDebug(const char *trigger, const char *msg, Allocator *pAlloc);

	//! print out single \em any with \em msg when \em trigger is activated independent of other trigger scopes
	/*! \param trigger scope of trigger to check for
		\param any (RO)Anything to print out
		\param msg message to print out
		\param pAlloc Allocator to use for allocating memory */
	static void AnythingWDDebug(const char *trigger, const ROAnything &any, const char *msg, Allocator *pAlloc);

	//! Check if \em trigger is activated
	/*! \param trigger scope of trigger to check for
		\return true if trigger is active
		\return false otherwise */
	static bool CheckWDDebug(const char *trigger);

	//! Accessor to exchange underlying debug configuration file
	/*! \param filename if no filename is given, just reload the default config file */
	static void ExchangeConfigFile(const char *filename = 0);

private:
	//! pointer to character buffer storing the trigger
	const char *fTrigger;
	//! flag to store if main trigger is enabled or not
	bool fTriggered;
	//! pointer to optional message to print out when entering/leaving a Trace scope
	const char *fpMsg;
	//! pointer to Allocator to use for allocating memory of internally used String buffers
	Allocator *fpAlloc;
};

/*! Macro to start a trace block using trigger string \em trigger
	\param trigger Will internally be used to do an Anything::LookupPath() search inside the \b Dbg.any file to check if trace output should be enabled or not
\code
	StartTrace(Server.Load);
\endcode
Will print out following messages
\code
	Server.Load: --- entering ---
		<more trace output of this method or enabled submethods>
	Server.Load: --- leaving ---
\endcode */
#define StartTrace(trigger) \
	Tracer recart(_QUOTE_(trigger)); recart.Use()

/*! Macro to start a trace block using trigger string \em trigger and additional message msg
	\param trigger Will internally be used to do an Anything::LookupPath() search inside the \b Dbg.any file to check if trace output should be enabled or not
	\param msg additional message to print out when entering and leaving
\code
	StartTrace1(Server.Load, "server in command [" << GetName() << "]");
\endcode
Will print out following messages
\code
	Server.Load: server in command [SomeServer] --- entering ---
		<more trace output of this method or enabled submethods>
	Server.Load: server in command [SomeServer] --- leaving ---
\endcode */
#define StartTrace1(trigger, msg) \
	String gsMrotcurtsnoCrecart(Coast::Storage::Current()); \
	Tracer recart(_QUOTE_(trigger), gsMrotcurtsnoCrecart << msg); recart.Use()

/*! Macro to print out a \em msg when surrounding StartTrace() trigger is enabled
	\param msg message to print out  */
#define Trace(msg) \
{ \
	String gsMecart(Coast::Storage::Current()); \
	recart.WDDebug(gsMecart << msg); \
}

/*! Macro to print out a message buffer \em buf with size \em sz when surrounding StartTrace() trigger is enabled
	\param buf pointer to buffer to print out
	\param sz size of buffer */
#define TraceBuf(buf, sz) \
{ \
	String gsMecart("\n\n<",-1, Coast::Storage::Current()); \
	gsMecart.Append((const void*)buf, sz).Append(">\n\n"); \
	recart.WDDebug(gsMecart); \
}

/*! Macro to print out a \em any when surrounding StartTrace() trigger is enabled
	\param any Anything to print out
	\param msg message to additionally print out */
#define TraceAny(any, msg) \
{ \
	String gsMecart(Coast::Storage::Current()); \
	recart.AnyWDDebug(any, gsMecart << msg); \
}

/*! Macro to print out a \em msg when surrounding StartTrace() trigger and the \em subtrigger is enabled
	\param subtrigger additional sublevel within current trigger scope
	\param msg message to print out */
#define SubTrace(subtrigger, msg) \
{ \
	String gsMecart(Coast::Storage::Current()); \
	recart.SubWDDebug(_QUOTE_(subtrigger), gsMecart << msg); \
}

/*! Macro to print out a message buffer \em buf with size \em sz when surrounding StartTrace() trigger and the \em subtrigger is enabled
	\param subtrigger additional sublevel within current trigger scope
	\param buf pointer to buffer to print out
	\param sz size of buffer */
#define SubTraceBuf(subtrigger, buf, sz) \
{ \
	String gsMecart("\n\n<",-1, Coast::Storage::Current()); \
	gsMecart.Append((const void*)buf, sz).Append(">\n\n"); \
	recart.SubWDDebug(_QUOTE_(subtrigger), gsMecart); \
}

/*! Macro to print out a \em any when surrounding StartTrace() trigger and the \em subtrigger is enabled
	\param subtrigger additional sublevel within current trigger scope
	\param any Anything to print out
	\param msg message to additionally print out */
#define SubTraceAny(subtrigger, any, msg) \
{ \
	String gsMecart(Coast::Storage::Current()); \
	recart.SubAnyWDDebug(_QUOTE_(subtrigger), any, gsMecart << msg); \
}

/*! Macro to print out a \em msg when \em trigger is enabled, this method is independent from StartTrace()
	\param trigger scope of trigger to check for
	\param msg message to print out
	\param allocator Allocator to use for allocating memory */
#define StatTrace(trigger, msg, allocator) \
{ \
	String gsMecart(allocator); \
	Tracer::StatWDDebug(_QUOTE_(trigger), gsMecart << msg, allocator); \
}

/*! Macro to print out a message buffer \em buf with size \em sz when \em trigger is enabled, this method is independent from StartTrace()
	\param trigger scope of trigger to check for
	\param buf pointer to buffer to print out
	\param sz size of buffer
	\param allocator Allocator to use for allocating memory */
#define StatTraceBuf(trigger, buf, sz, allocator) \
{ \
	String gsMecart("\n\n<",-1, allocator); \
	gsMecart.Append((const void*)buf, sz).Append(">\n\n"); \
	Tracer::StatWDDebug(_QUOTE_(trigger), gsMecart, allocator); \
}

/*! Macro to print out a \em any when \em trigger is enabled, this method is independent from StartTrace()
	\param trigger scope of trigger to check for
	\param any Anything to print out
	\param msg message to additionally print out
	\param allocator Allocator to use for allocating memory */
#define StatTraceAny(trigger, any, msg, allocator) \
{ \
	String gsMecart(allocator); gsMecart << msg; \
	Tracer::AnythingWDDebug(_QUOTE_(trigger), any, gsMecart, allocator); \
}

/*! helper to check if trigger is enabled
	\param trigger scope of trigger to check for */
#define TriggerEnabled(trigger) \
	Tracer::CheckWDDebug(_QUOTE_(trigger))

#else
//--- optimized versions of the debug macros they expand into nothing
// constructor
#define StartTrace(trigger)
#define StartTrace1(trigger, msg)
// debug statements
#define TraceBuf(buf, sz)
#define Trace(msg)
#define TraceAny(any, msg)
// subdebugs
#define SubTrace(subtrigger, msg)
#define SubTraceAny(subtrigger, any, msg)
#define SubTraceBuf(subtrigger, buf, sz)
// static trace facility
#define StatTrace(trigger, msg, allocator)
#define StatTraceBuf(trigger, buf, sz, allocator)
#define StatTraceAny(trigger, any, msg, allocator)
// helper to check if we are triggered
#define TriggerEnabled(trigger)		false
#endif

#endif		//not defined _Dbg_H
