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
#include "Threads.h"

//--- c-library modules used ---------------------------------------------------

//---- TimeLoggingModule ---------------------------------------------------------------
RegisterModule(TimeLoggingModule);

bool TimeLoggingModule::fgDoTiming = false;
bool TimeLoggingModule::fgDoLogging = false;
const char *TimeLoggingModule::fgpLogEntryBasePath = "Log.Times";

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
	// set defaults
	fgDoTiming = fgDoLogging = false;
	if ( ((ROAnything)config).LookupPath(roaModuleConfig, fName) ) {
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
TimeLoggerEntry::TimeLoggerEntry(const char *pSection, const char *pKey, String &msg, Context &ctx, TimeLogger::eResolution aResolution)
	: fLogger( pSection, pKey, msg, ctx, aResolution)
{
}

TimeLoggerEntry::~TimeLoggerEntry()
{
}

//---- TimeLogger --------------------------------------------------------------------------
TimeLogger::TimeLogger(const char *pSection, const char *pKey, const String &msg, Context &ctx, TimeLogger::eResolution aResolution)
	: fMsgStr(msg)
	, fpSection( pSection )
	, fpKey( pKey )
	, fContext(ctx)
	, fDiffTimer( (DiffTimer::eResolution)aResolution )
	, fpcUnit( aResolution == TimeLogger::eMilliseconds ? "ms" : ( aResolution == TimeLogger::eMicroseconds ? "us" : ( aResolution == TimeLogger::eNanoseconds ? "ns" : "s" ) ) )
	, fanyNestingLevel( fContext.Lookup("TimeLoggerNestingLevel", -1L) + 1L )
	, fEntry(fContext, "nesting", fanyNestingLevel, "TimeLoggerNestingLevel")
{
}

TimeLogger::~TimeLogger()
{
	if ( TimeLoggingModule::fgDoTiming ) {
		DiffTimer::tTimeType lDiffTime( fDiffTimer.Diff() );
		ROAnything roaBase, roaSection, roaValue;
		if ( fContext.Lookup("EnabledValues", roaBase ) && roaBase.LookupPath( roaSection, fpSection ) && roaSection.LookupPath( roaValue, fpKey ) && roaValue.AsBool(false) ) {
			Anything data;
			if ( fContext.Lookup("SimulatedValues", roaBase) && roaBase.LookupPath( roaSection, fpSection ) && roaSection.LookupPath( roaValue, fpKey ) ) {
				lDiffTime = roaValue.AsLong(-1L);
			}
			String strLogTimeKey( TimeLoggingModule::fgpLogEntryBasePath );
			strLogTimeKey.Append('.').Append(fpSection).Append('.').Append(fpKey);
			data["Time"] = (long)lDiffTime;
			data["Msg"] = fMsgStr;
			data["Unit"] = fpcUnit;
			data["ThreadId"] = Thread::MyId();
			data["NestingLevel"] = fanyNestingLevel;
			StatTraceAny(TimeLogger.~TimeLogger, data, "ENABLED  Section <" << fpSection << "> Key <" << fpKey << "> Message <" << fMsgStr << ">", Storage::Current());
			StorePutter::Operate(data, fContext, "", strLogTimeKey, true);
		} else {
			StatTrace(TimeLogger.~TimeLogger, "DISABLED Section <" << fpSection << "> Key <" << fpKey << ">", Storage::Current());
		}
	}
}
