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
		ROAnything roaEntryBase, roaSelectedEntries;
		if ( ctx.Lookup( TimeLoggingModule::fgpLogEntryBasePath, roaEntryBase ) ) {
			String strKey( config["TimeEntries"].AsString() );
			if ( strKey.Length() ) {
				bRet = roaEntryBase.LookupPath( roaSelectedEntries, strKey );
			} else {
				roaSelectedEntries = roaEntryBase;
				bRet = true;
			}
			if ( bRet ) {
				TraceAny(roaSelectedEntries, "Entries");
				String key;
				bRet = GenLogEntries(key, roaSelectedEntries, ctx, channel, (AppLogModule::eLogLevel)config["Severity"].AsLong((long)AppLogModule::eINFO));
			}
		}
	}
	return bRet;
}

bool TimeLoggingAction::GenLogEntries(const String &entryPath, const ROAnything &entry, Context &ctx, const String &channel, AppLogModule::eLogLevel iLevel)
{
	StartTrace(TimeLoggingAction.GenLogEntries);
	long entriesSz( entry.GetSize() );

	if ( entry.SlotName(0L) ) { // it is an anonymous array
		// assume a substructure we traverse recursively
		String path;
		for (long i = entriesSz - 1; i >= 0; --i) {
			if ( entryPath.Length() > 0 ) {
				path = entryPath;
				path.Append('.');
			} else {
				path.Trim(0L);
			}
			path.Append( entry.SlotName(i) );
			Trace("Path: <" << path << ">");
			if (!GenLogEntries(path, entry[i], ctx, channel, iLevel)) {
				return false;
			}
		}
	} else {
		// assume an array of timing entries
		for (long i = entriesSz - 1; i >= 0; --i) {
			if (!GenLogEntry(entryPath, entry[i], ctx, channel, iLevel)) {
				return false;
			}
		}
	}
	return true;
}

bool TimeLoggingAction::GenLogEntry(const String &key, const ROAnything &entry, Context &ctx, const String &channel, AppLogModule::eLogLevel iLevel)
{
	StatTraceAny(TimeLoggingAction.GenLogEntry, entry, "Entries: " << (long)entry.GetSize(), Storage::Current() );
	Anything anyKey;
	anyKey["Key"] = key;
	Context::PushPopEntry<ROAnything> aValuesEntry( ctx, "LoggingEntries", entry, "TimeLogEntry" );
	Context::PushPopEntry<Anything> aKeyEntry( ctx, "LoggingEntryKey", anyKey, "TimeLogEntry" );
	return AppLogModule::Log(ctx, channel, iLevel);
}
