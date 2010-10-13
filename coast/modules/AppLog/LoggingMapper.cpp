/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LoggingMapper.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "AppLog.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- LoggingMapper ------------------------------------------------------------------
RegisterResultMapper(LoggingMapper);

LoggingMapper::LoggingMapper(const char *name)
	: ResultMapper(name)
{
	StartTrace(LoggingMapper.Ctor);
}

IFAObject *LoggingMapper::Clone(Allocator *a) const
{
	return new (a) LoggingMapper(fName);
}

bool LoggingMapper::DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script)
{
	StartTrace1(LoggingMapper.DoPutAny, NotNull(key));
	String channel( Lookup("Channel", "") );
	Trace("Channel: <" << channel << ">");
	if ( channel.Length() ) {
		TraceAny(value, "value for key <" << key << ">");
		Context::PushPopEntry<ROAnything> aEntry(ctx, "LoggingMapperData", value, key);
		ROAnything roaKey;
		if ( Lookup("Format", roaKey) ) {
			TraceAny(roaKey, "format spec for logging key [" << key << "]");
			return AppLogModule::Log(ctx, channel, roaKey, (AppLogModule::eLogLevel)Lookup("Severity").AsLong((long)AppLogModule::eINFO));
		} else {
			return AppLogModule::Log(ctx, channel, (AppLogModule::eLogLevel)Lookup("Severity").AsLong((long)AppLogModule::eINFO));
		}
	}
	return false;
}
