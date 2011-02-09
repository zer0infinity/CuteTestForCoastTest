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

//---- TimeLoggingModule ---------------------------------------------------------------
RegisterModule(TimeLoggingModule);

bool TimeLoggingModule::fgDoTiming = false;
bool TimeLoggingModule::fgDoLogging = false;
const char *TimeLoggingModule::fgpLogEntryBasePath = "Log.Times";
THREADKEY TimeLoggingModule::fgNestingLevelKey = 0;
bool TimeLoggingModule::fgTLSUsable = false;

//:perform close and destruction of OraclePooledConnection in thread specific storage
static class ThreadSpecificNestingLevelCleaner : public CleanupHandler
{
protected:
	//:destruct nesting level
	virtual bool DoCleanup() {
		StatTrace(ThreadSpecificNestingLevelCleaner.DoCleanup, "ThrdId: " << Thread::MyId(), Coast::Storage::Global());
		long *pLevel = 0;
		if (GETTLSDATA(TimeLoggingModule::fgNestingLevelKey, pLevel, long)) {
			delete pLevel;
			pLevel = 0;
			if (SETTLSDATA(TimeLoggingModule::fgNestingLevelKey, pLevel)) {
				return true;
			}
		}
		return false;
	}
} fgTSNLCleaner;

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
		if ( THRKEYCREATE(TimeLoggingModule::fgNestingLevelKey, 0) != 0 ) {
			SYSWARNING("TlsAlloc of TimeLoggingModule::fgNestingLevelKey failed");
		} else {
			fgTLSUsable = true;
		}
		return true;
	}
	return false;
}

bool TimeLoggingModule::Finis()
{
	StartTrace(TimeLoggingModule.Finis);
	if ( fgTLSUsable ) {
		if (THRKEYDELETE(TimeLoggingModule::fgNestingLevelKey) != 0) {
			SYSERROR("TlsFree of TimeLoggingModule::fgNestingLevelKey failed" );
		}
	}
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
	: fpLogger(0)
{
	if ( TimeLoggingModule::fgDoTiming ) {
		fpLogger = TimeLoggerPtr(new (Coast::Storage::Current()) TimeLogger( pSection, pKey, msg, ctx, aResolution ));
	}
}

TimeLoggerEntry::~TimeLoggerEntry()
{}

//---- TimeLogger --------------------------------------------------------------------------
TimeLogger::TimeLogger(const char *pSection, const char *pKey, const String &msg, Context &ctx, TimeLogger::eResolution aResolution)
	: fMsgStr(msg)
	, fpSection( pSection )
	, fpKey( pKey )
	, fContext(ctx)
	, fDiffTimer( (DiffTimer::eResolution)aResolution )
	, fpcUnit( aResolution == TimeLogger::eMilliseconds ? "ms" : ( aResolution == TimeLogger::eMicroseconds ? "us" : ( aResolution == TimeLogger::eNanoseconds ? "ns" : "s" ) ) )
{
	if ( TimeLoggingModule::fgTLSUsable ) {
		long *pNestingLevel(0);
		if ( !GETTLSDATA(TimeLoggingModule::fgNestingLevelKey, pNestingLevel, long) ) {
			Thread::RegisterCleaner(&fgTSNLCleaner);
			pNestingLevel = new long;
			*pNestingLevel = -1L;
		}
		++(*pNestingLevel);
		SETTLSDATA(TimeLoggingModule::fgNestingLevelKey, pNestingLevel);
	}
}

TimeLogger::~TimeLogger()
{
	DiffTimer::tTimeType lDiffTime( fDiffTimer.Diff() );
	ROAnything roaBase, roaSection, roaValue;
	long lNestingLevel(0L);
	if ( TimeLoggingModule::fgTLSUsable ) {
		long *pNestingLevel(0);
		if ( GETTLSDATA(TimeLoggingModule::fgNestingLevelKey, pNestingLevel, long) ) {
			lNestingLevel = (*pNestingLevel)--;
			SETTLSDATA(TimeLoggingModule::fgNestingLevelKey, pNestingLevel);
		}
	}
	if ( fContext.Lookup("EnabledValues", roaBase ) && roaBase.LookupPath( roaSection, fpSection ) && roaSection.LookupPath( roaValue, fpKey ) && roaValue.AsBool(false) ) {
		Anything data;
		if ( fContext.Lookup("SimulatedValues", roaBase) && roaBase.LookupPath( roaSection, fpSection ) && roaSection.LookupPath( roaValue, fpKey ) ) {
			lDiffTime = roaValue.AsLong(-1L);
		}
		data[eSection] = fpSection;
		data[eKey] = fpKey;
		data[eTime] = (long)lDiffTime;
		data[eMsg] = fMsgStr;
		data[eUnit] = fpcUnit;
		data[eNestingLevel] = lNestingLevel;
		StatTraceAny(TimeLogger.~TimeLogger, data, "ENABLED  Section <" << fpSection << "> Key <" << fpKey << "> Message <" << fMsgStr << ">", Coast::Storage::Current());
		StorePutter::Operate(data, fContext, "", TimeLoggingModule::fgpLogEntryBasePath, true);
	} else {
		StatTrace(TimeLogger.~TimeLogger, "DISABLED Section <" << fpSection << "> Key <" << fpKey << ">", Coast::Storage::Current());
	}
}
