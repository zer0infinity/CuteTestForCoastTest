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
#include "DiffTimer.h"
#include "Context.h"
#include "WDModule.h"

//---- TimeLoggingModule ----------------------------------------------------------
//! <B>Module to enable/disable timing and logging of methods</B>
/*!
<B>Configuration:</B><PRE>
{
	/DoTiming		long	optional, default 0 (false), timing of methods will be enabled if set to true (>0)
	/DoLogging		long	optional, default 0 (false), logging of collected timing information will be anabled if set to true (>0)
}</PRE>
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
	~TimeLoggingModule();

	//! implementers should initialize module using config
	virtual bool Init(const Anything &config);
	//! implementers should terminate module expecting destruction
	virtual bool Finis();
	//! initializes module after termination for reinitialization; default uses Init; check if this applies
	virtual bool ResetInit(const Anything &config);
	//! terminates module for reinitialization; default uses Finis; check if this applies
	virtual bool ResetFinis(const Anything &config);

	static bool fgDoTiming;
	static bool fgDoLogging;
};

class TimeLogger;

//---- TimeLoggerEntry --------------------------------------------------------------------------
//! <B>Abstracting object to instantiate a TimeLogger object only if global TimeLoggingModule::fgDoTiming is enabled</B>
/*!
This level of abstraction is needed to reduce the overhead of always instantiating a DiffTimer object as done in TimeLogger.
It is also important to have because there is no flexible other way to have time logging enabled/disabled on demand without recompiling everytime.
*/
class EXPORTDECL_WDBASE TimeLoggerEntry
{
public:
	TimeLoggerEntry(const char *pSection, const char *key, String &msg, Context &ctx);
	~TimeLoggerEntry();

	//!dummy method to prevent optimization of unused variables
	void Use() const { }

private:
	TimeLogger	*fpLogger;
	String		*fpKey;
};

#define MethodTimer(key,msg,ctx)	\
	String gsMreggoLemiT;			\
	TimeLoggerEntry yrtnEreggoLemiT("Method.", _QUOTE_(key), gsMreggoLemiT << msg, ctx); yrtnEreggoLemiT.Use()

#define DAAccessTimer(key,msg,ctx)	\
	String gsMreggoLemiT;			\
	TimeLoggerEntry yrtnEreggoLemiT("DataAccess.", _QUOTE_(key), gsMreggoLemiT << msg, ctx); yrtnEreggoLemiT.Use()

#define RequestTimer(key,msg,ctx)	\
	String gsMreggoLemiT;			\
	TimeLoggerEntry yrtnEreggoLemiT("Request.", _QUOTE_(key), gsMreggoLemiT << msg, ctx); yrtnEreggoLemiT.Use()

#define RequestTimeLogger(ctx)						\
	if ( TimeLoggingModule::fgDoLogging )			\
	{												\
		String snarTreggoLemiT("TimeLoggingAction");\
		ctx.Process(snarTreggoLemiT);				\
	}

//---- TimeLogger --------------------------------------------------------------------------
//!helper class to log timing information
class EXPORTDECL_WDBASE TimeLogger
{
	friend class TimeLoggerEntry;
protected:
	//!starts the timer for request nr
	TimeLogger(const String &key, const String &msg, Context &ctx);
	//!stops the timer and prints the results for this request and thread
	~TimeLogger();

	//!message string printed in the destructor
	const String &fMsgStr;
	//!key that defines the place where the info is stored
	const String &fKey;
	//!log into the context
	Context &fContext;
	//!timer to measure elapsed time
	DiffTimer fDiffTimer;

private:
	//!do not use
	TimeLogger();
	//!do not use
	TimeLogger(const TimeLogger &);
	//!do not use
	TimeLogger &operator=(TimeLogger &);
};

#endif
