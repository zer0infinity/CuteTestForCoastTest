/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LoggingAction.h"

//--- standard modules used ----------------------------------------------------
#include "Timers.h"
#include "Renderer.h"
#include "Dbg.h"
#include "AnyIterators.h"

//---- LoggingAction ---------------------------------------------------------------
RegisterAction(LoggingAction);

LoggingAction::LoggingAction(const char *name)
	: Action(name) { }

LoggingAction::~LoggingAction() { }

bool LoggingAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(LoggingAction.DoExecAction);
	bool bRet = false;
	String channel = config["Channel"].AsString("");
	if ( channel.Length() ) {
		Trace("Channel: <" << channel << ">");
		ROAnything roaFormat;
		if ( config.LookupPath( roaFormat, "Format" ) ) {
			bRet = AppLogModule::Log(ctx, channel, roaFormat, (AppLogModule::eLogLevel)config["Severity"].AsLong((long)AppLogModule::eINFO));
		} else {
			bRet = AppLogModule::Log(ctx, channel, (AppLogModule::eLogLevel)config["Severity"].AsLong((long)AppLogModule::eINFO));
		}
	}
	return bRet;
}

//---- TimeLoggingAction ---------------------------------------------------------------

RegisterAction(TimeLoggingAction);

TimeLoggingAction::TimeLoggingAction(const char *name) : Action(name) { }

TimeLoggingAction::~TimeLoggingAction() { }

bool TimeLoggingAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(TimeLoggingAction.DoExecAction);
	bool bRet( false );
	String channel( config["Channel"].AsString() );
	if ( channel.Length() ) {
		Trace("Channel: <" << channel << ">");
		ROAnything roaEntryBase;
		if ( ctx.Lookup( TimeLoggingModule::fgpLogEntryBasePath, roaEntryBase ) ) {
			String strSection( config["TimeEntries"].AsString() );
			Trace("section [" << (strSection.Length() ? strSection : "*") << "]");
			bRet = GenLogEntries(strSection, roaEntryBase, ctx, channel, (AppLogModule::eLogLevel)config["Severity"].AsLong((long)AppLogModule::eINFO));
		}
	}
	return bRet;
}

bool TimeLoggingAction::GenLogEntries(const String &strSection, const ROAnything &entry, Context &ctx, const String &channel, AppLogModule::eLogLevel iLevel)
{
	StartTrace1(TimeLoggingAction.GenLogEntries, "section [" << (strSection.Length() ? strSection : "*") << "]");
	AnyExtensions::Iterator<ROAnything> aEntryIter(entry);
	ROAnything roaEntry;
	while ( aEntryIter.Next(roaEntry) ) {
		// structure of entry:
		//	/Section
		//	/Key
		//	/Time
		//	/Msg
		//	/Unit
		//	/NestingLevel
		TraceAny(roaEntry, "current entry");
		if ( strSection.Length() == 0 || strSection.IsEqual(roaEntry["Section"].AsCharPtr()) ) {
			// override key using the section prefix if any
			String strKey( roaEntry[TimeLogger::eSection].AsString() );
			if ( strKey.Length() > 0 ) {
				strKey.Append('.');
			}
			Anything anyKey;
			anyKey["Section"] = roaEntry[TimeLogger::eSection].AsString();
			anyKey["Time"] = roaEntry[TimeLogger::eTime].AsLong();
			anyKey["Msg"] = roaEntry[TimeLogger::eMsg].AsString();
			anyKey["Unit"] = roaEntry[TimeLogger::eUnit].AsString();
			anyKey["NestingLevel"] = roaEntry[TimeLogger::eNestingLevel].AsLong();
			anyKey["Key"] = String(strKey).Append( roaEntry[TimeLogger::eKey].AsString() );
			anyKey["ThreadId"] = Thread::MyId();
			Context::PushPopEntry<Anything> aKeyEntry( ctx, "LoggingEntryKey", anyKey, "TimeLogEntry" );
			if ( !AppLogModule::Log(ctx, channel, iLevel) ) {
				return false;
			}
		}
	}
	return true;
}
