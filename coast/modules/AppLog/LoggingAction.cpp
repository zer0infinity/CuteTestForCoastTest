/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LoggingAction.h"
#include "Timers.h"
#include "AnyIterators.h"

RegisterAction(LoggingAction);

bool LoggingAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(LoggingAction.DoExecAction);
	bool bRet = false;
	String channel = config["Channel"].AsString("");
	if (channel.Length()) {
		Trace("Channel: <" << channel << ">");
		ROAnything roaFormat;
		if (config.LookupPath(roaFormat, "Format")) {
			bRet = AppLogModule::Log(ctx, channel, roaFormat, (AppLogModule::eLogLevel) config["Severity"].AsLong((long) AppLogModule::eINFO));
		} else {
			bRet = AppLogModule::Log(ctx, channel, (AppLogModule::eLogLevel) config["Severity"].AsLong((long) AppLogModule::eINFO));
		}
	}
	return bRet;
}

RegisterAction(TimeLoggingAction);

bool TimeLoggingAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
	StartTrace(TimeLoggingAction.DoExecAction);
	bool bRet(false);
	String channel(config["Channel"].AsString());
	if (channel.Length()) {
		Trace("Channel: <" << channel << ">");
		ROAnything roaEntryBase;
		if (ctx.Lookup(TimeLoggingModule::fgpLogEntryBasePath, roaEntryBase)) {
			String strSection(config["TimeEntries"].AsString());
			Trace("section [" << (strSection.Length() ? strSection : "*") << "]");
			bRet = GenLogEntries(strSection, roaEntryBase, ctx, channel,
					(AppLogModule::eLogLevel) config["Severity"].AsLong((long) AppLogModule::eINFO));
		}
	}
	return bRet;
}

bool TimeLoggingAction::GenLogEntries(const String &strSection, const ROAnything &entry, Context &ctx, const String &channel,
		AppLogModule::eLogLevel iLevel) {
	StartTrace1(TimeLoggingAction.GenLogEntries, "section [" << (strSection.Length() ? strSection : "*") << "]");
	AnyExtensions::Iterator<ROAnything> aEntryIter(entry);
	ROAnything roaEntry;
	while (aEntryIter.Next(roaEntry)) {
		TraceAny(roaEntry, "current entry");
		if (strSection.Length() == 0 || strSection.IsEqual(roaEntry[TimeLogger::eSection].AsString())) {
			Anything anyKey;
			anyKey["Section"] = roaEntry[TimeLogger::eSection].AsString();
			anyKey["Key"] = roaEntry[TimeLogger::eKey].AsString();
			anyKey["Time"] = roaEntry[TimeLogger::eTime].AsLong();
			anyKey["Unit"] = roaEntry[TimeLogger::eUnit].AsString();
			anyKey["Msg"] = roaEntry[TimeLogger::eMsg].AsString();
			anyKey["NestingLevel"] = roaEntry[TimeLogger::eNestingLevel].AsLong();
			anyKey["ThreadId"] = Thread::MyId();
			Context::PushPopEntry<Anything> aKeyEntry(ctx, "LoggingEntryKey", anyKey, "TimeLogEntry");
			if (!AppLogModule::Log(ctx, channel, iLevel)) {
				return false;
			}
		}
	}
	return true;
}
