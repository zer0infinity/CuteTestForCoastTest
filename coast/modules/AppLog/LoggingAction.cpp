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
		bRet = AppLogModule::Log(ctx, channel, (AppLogModule::eLogLevel)config["Severity"].AsLong((long)AppLogModule::eINFO));
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
	bool bRet = false;
	String channel = config["Channel"].AsString("");
	if ( channel.Length() ) {
		Trace("Channel: <" << channel << ">");
		String entriesPath;
		Renderer::RenderOnString(entriesPath, ctx, config["TimeEntries"]);
		Trace("TimeEntriesPath: <" << entriesPath << ">");
		if (entriesPath.Length() == 0) {
			entriesPath = "Log.Times.Request";
		}
		ROAnything timingEntries(ctx.Lookup(entriesPath));
		TraceAny(timingEntries, "Entries: ");
		String key;
		bRet = GenLogEntries(key, timingEntries, ctx, channel, (AppLogModule::eLogLevel)config["Severity"].AsLong((long)AppLogModule::eINFO));
	}
	return bRet;
}

bool TimeLoggingAction::GenLogEntries(const String &entryPath, const ROAnything &entry, Context &ctx, const String &channel, AppLogModule::eLogLevel iLevel)
{
	StartTrace(TimeLoggingAction.GenLogEntries);
	long entriesSz = entry.GetSize();

	if ( entry.SlotName(0L) ) { // it is an anonymous array
		// assume a substructure we traverse recursively
		String path;
		for (long i = 0; i < entriesSz; ++i) {
			if ( entryPath.Length() > 0 ) {
				path = entryPath;
				path << ".";
			} else {
				path = "";
			}
			path << entry.SlotName(i);
			Trace("Path: <" << path << ">");
			if (!GenLogEntries(path, entry[i], ctx, channel, iLevel)) {
				return false;
			}
		}
	} else {
		// assume an array of timing entries
		for (long i = 0; i < entriesSz; ++i) {
			if (!GenLogEntry(entryPath, entry[i], ctx, channel, iLevel)) {
				return false;
			}
		}
	}
	return true;
}

bool TimeLoggingAction::GenLogEntry(const String &key, const ROAnything &entry, Context &ctx, const String &channel, AppLogModule::eLogLevel iLevel)
{
	StartTrace(TimeLoggingAction.GenLogEntry);
	Trace("Entry Size: " << (long)entry.GetSize());
	TraceAny(entry, "Entries: ");

	Anything logEntry;
	logEntry = entry.DeepClone();
	logEntry["Key"] = key;

	TraceAny(logEntry, "LogEntry added: ");
	ctx.GetTmpStore()["TimeLogEntry"] = logEntry;
	if (!AppLogModule::Log(ctx, channel, iLevel)) {
		return false;
	}
	return true;
}
