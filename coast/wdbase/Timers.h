/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TIMERS_H
#define _TIMERS_H

#include "config_wdbase.h"
#include "AllocatorNewDelete.h"
#include "DiffTimer.h"
#include "Context.h"
#include "WDModule.h"
#include "Threads.h"

//---- TimeLoggingModule ----------------------------------------------------------
//! <B>Module to enable/disable timing and logging of methods</B>
/*!
\par Configuration
\code
{
	/DoTiming		long	optional, default 0 (false), timing of methods will be enabled if set to true (>0)
	/DoLogging		long	optional, default 0 (false), logging of collected timing information will be anabled if set to true (>0)
}
\endcode

To use this functionality, you have to add, for example, the MethodTimer macro into the scope you want to gather timing information.
The timing information will be stored in the TmpStore of the given context for later logging.
To enable logging, the RequestTimeLogger macro has to be put before destroying the context.
Currently, this call is already present in RequestProcessor to gather Request time informations by default.

But unless you declare the TimeLoggingModule, and configure it to DoTiming and DoLogging nothing will be done - except allocation and destruction of the message String.
*/
class EXPORTDECL_WDBASE TimeLoggingModule : public WDModule
{
public:
	//--- constructors
	TimeLoggingModule(const char *name);
	virtual ~TimeLoggingModule();

	//! implementers should initialize module using config
	virtual bool Init(const ROAnything config);
	//! implementers should terminate module expecting destruction
	virtual bool Finis();
	//! initializes module after termination for reinitialization; default uses Init; check if this applies
	virtual bool ResetInit(const ROAnything config);
	//! terminates module for reinitialization; default uses Finis; check if this applies
	virtual bool ResetFinis(const ROAnything config);

	static bool fgDoTiming;
	static bool fgDoLogging;
	static const char *fgpLogEntryBasePath;
	static THREADKEY fgNestingLevelKey;
	static bool fgTLSUsable;
};

//---- TimeLogger --------------------------------------------------------------------------
//!helper class to log timing information
class EXPORTDECL_WDBASE TimeLogger : public Coast::AllocatorNewDelete
{
	friend class TimeLoggerEntry;
public:
	enum eResolution {
		eSeconds = DiffTimer::eSeconds,
		eMilliseconds = DiffTimer::eMilliseconds,
		eMicroseconds = DiffTimer::eMicroseconds,
		eNanoseconds = DiffTimer::eNanoseconds
	};
	enum eTimeEntryMap {
		eSection,
		eKey,
		eTime,
		eUnit,
		eMsg,
		eNestingLevel,
	};
	//!starts the timer for request nr
	TimeLogger(const char *pSection, const char *pKey, const String &msg, Context &ctx, TimeLogger::eResolution aResolution);
	//!stops the timer and prints the results for this request and thread
	~TimeLogger();

protected:
	//!message string printed in the destructor
	const String &fMsgStr;
	//!key that defines the place where the info is stored
	const char	*fpSection, *fpKey;
	//!log into the context
	Context &fContext;
	//!timer to measure elapsed time
	DiffTimer fDiffTimer;
	//! store unit
	const char *fpcUnit;

private:
	//!do not use
	TimeLogger();
	//!do not use
	TimeLogger(const TimeLogger &);
	//!do not use
	TimeLogger &operator=(TimeLogger &);
};

//---- TimeLoggerEntry --------------------------------------------------------------------------
//! <B>Abstracting object to instantiate a TimeLogger object only if global TimeLoggingModule::fgDoTiming is enabled</B>
/*!
This level of abstraction is needed to reduce the overhead of always instantiating a DiffTimer object as done in TimeLogger.
It is also important to have because there is no flexible other way to have time logging enabled/disabled on demand without recompiling everytime.
*/
class EXPORTDECL_WDBASE TimeLoggerEntry
{
public:
	typedef std::auto_ptr<TimeLogger> TimeLoggerPtr;
	TimeLoggerEntry(const char *pSection, const char *pKey, String &msg, Context &ctx, TimeLogger::eResolution aResolution);
	~TimeLoggerEntry();

private:
	TimeLoggerPtr fpLogger;
};

#define MethodTimer(key, msg, ctx)	\
	MethodTimerName(key, msg, ctx, __LINE__)

#define MethodTimerName(key, msg, ctx, name)	\
	String _NAME2_(gsMreggoLemiT,name);			\
	TimeLoggerEntry _NAME2_(yrtnEreggoLemiT,name)("Method", _QUOTE_(key), _NAME2_(gsMreggoLemiT,name) << msg, ctx, TimeLogger::eMilliseconds)

#define MethodTimerUnit(key, msg, ctx, res)	\
	MethodTimerUnitName(key, msg, ctx, res, __LINE__)

#define MethodTimerUnitName(key, msg, ctx, res, name)	\
	String _NAME2_(gsMreggoLemiT,name);			\
	TimeLoggerEntry _NAME2_(yrtnEreggoLemiT,name)("Method", _QUOTE_(key), _NAME2_(gsMreggoLemiT,name) << msg, ctx, res)

#define DAAccessTimer(key,msg,ctx)	\
	DAAccessTimerName(key, msg, ctx, __LINE__)

#define DAAccessTimerName(key,msg,ctx,name)	\
	String _NAME2_(gsMreggoLemiT,name);			\
	TimeLoggerEntry _NAME2_(yrtnEreggoLemiT,name)("DataAccess", _QUOTE_(key), _NAME2_(gsMreggoLemiT,name) << msg, ctx, TimeLogger::eMilliseconds)

#define RequestTimer(key,msg,ctx)	\
	RequestTimerName(key, msg, ctx, __LINE__)

#define RequestTimerName(key,msg,ctx,name)	\
	String _NAME2_(gsMreggoLemiT,name);			\
	TimeLoggerEntry _NAME2_(yrtnEreggoLemiT,name)("Request", _QUOTE_(key), _NAME2_(gsMreggoLemiT,name) << msg, ctx, TimeLogger::eMilliseconds)

#define RequestTimeLogger(ctx)						\
	if ( TimeLoggingModule::fgDoLogging )			\
	{												\
		String snarTreggoLemiT("TimeLoggingAction");\
		ctx.Process(snarTreggoLemiT);				\
	}

#endif
