/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Timers.h"

//--- standard modules used ----------------------------------------------------
#include "AnythingUtils.h"
#include "Registry.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- TimeLoggingModule ---------------------------------------------------------------
RegisterModule(TimeLoggingModule);

bool TimeLoggingModule::fgDoTiming = false;
bool TimeLoggingModule::fgDoLogging = false;

TimeLoggingModule::TimeLoggingModule(const char *name)
	: WDModule(name)
{
	StartTrace(TimeLoggingModule.TimeLoggingModule);
}

TimeLoggingModule::~TimeLoggingModule()
{
}

bool TimeLoggingModule::Init(const ROAnything config)
{
	StartTrace(TimeLoggingModule.Init);
	SubTraceAny(FullConfig, config, "Config: ");
	ROAnything roaModuleConfig;
	if ( ((ROAnything)config).LookupPath(roaModuleConfig, fName) ) {
		// set defaults
		fgDoTiming = fgDoLogging = false;
		TraceAny(roaModuleConfig, "Module config");
		fgDoTiming = roaModuleConfig["DoTiming"].AsBool(false);
		// logging only makes sense if Timing enabled
		fgDoLogging = fgDoTiming && roaModuleConfig["DoLogging"].AsBool(false);
		return true;
	}
	return false;
}

bool TimeLoggingModule::Finis()
{
	StartTrace(TimeLoggingModule.Finis);
	return true;
}

bool TimeLoggingModule::ResetInit(const ROAnything config)
{
	StartTrace(TimeLoggingModule.ResetInit);
	return WDModule::ResetInit(config);
}

bool TimeLoggingModule::ResetFinis(const ROAnything config)
{
	StartTrace(TimeLoggingModule.ResetFinis);
	return WDModule::ResetFinis(config);
}

//---- TimeLoggerEntry --------------------------------------------------------------------------
TimeLoggerEntry::TimeLoggerEntry(const char *pSection, const char *key, String &msg, Context &ctx)
	: fpLogger(0)
	, fpKey(0)
{
	StartTrace(TimeLoggerEntry.TimeLoggerEntry);
	if ( TimeLoggingModule::fgDoTiming ) {
		fpKey = new String("Log.Times.");
		(*fpKey) << pSection << key;
		Trace("Key [" << (*fpKey) << "], Msg [" << msg << "]");
		fpLogger = new TimeLogger((*fpKey), msg, ctx);
	}
}

TimeLoggerEntry::~TimeLoggerEntry()
{
	StartTrace(TimeLoggerEntry.~TimeLoggerEntry);
	delete fpLogger;
	delete fpKey;
}

//---- TimeLogger --------------------------------------------------------------------------
TimeLogger::TimeLogger(const String &key, const String &msg, Context &ctx)
	: fMsgStr(msg)
	, fKey(key)
	, fContext(ctx)
{
	StartTrace(TimeLogger.Ctor);
	Trace("Key:<" << fKey << "> Message:<" << fMsgStr << ">");
}

TimeLogger::~TimeLogger()
{
	StartTrace1(TimeLogger.Dtor, "Key: <" << fKey << ">");
	String enablerKey("EnabledValues.");
	enablerKey << fKey;
	if (fContext.Lookup(enablerKey).AsBool(0L)) {
		Anything config;
		Anything data;
		Anything dataDest;
		String simulatedValue("SimulatedValues.");
		simulatedValue << fKey;

		config["Slot"] = fKey;
		data["Time"] = (long)fDiffTimer.Diff(fContext.Lookup(simulatedValue).AsLong(-1L));
		data["Msg"] = fMsgStr;
		StoreFinder::Operate(fContext, dataDest, config);
		dataDest.Append(data);
		TraceAny(data, "LogEntry");
	}
}
