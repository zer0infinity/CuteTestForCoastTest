/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LoggingMapper_H
#define _LoggingMapper_H

//---- ResultMapper include -------------------------------------------------
#include "Mapper.h"

//---- LoggingMapper ----------------------------------------------------------
//! <b>Mapper which logs messages on a configurable channel and routes message again</b>
/*!
\par Configuration
\code
{
	/Channel		String		mandatory, channel name to log to
	/Severity		long		optional, default AppLogModule::eINFO, Severity [CRITICAL=1, FATAL=2, ERROR=4, WARN=8, INFO=16, OK=32, MAINT=64, DEBUG=128], all levels lower_equal (<=) the specified value will get logged
	/Format {					optional, specify log format for mapped key, value will be temporarily available in context using <key> as base name for lookup
		...			Rendererspec
	}
}
\endcode
*/
class LoggingMapper : public ResultMapper
{
public:
	//--- constructors
	LoggingMapper(const char *name);
	//--- support for prototype
	IFAObject *Clone() const;

protected:
	//! Major hook for subclasses that want to do something with their config passed as script. The default is to interpret the script and put a value for every script item used. Recursion will be stopped by DoFinalPutAny which places its value under slot key below given DoGetDestinationSlot()
	/*! \param key the key usually defines the associated kind of output-value
		\param value the value to be mapped
		\param ctx the context of the invocation
		\param script current mapper configuration as ROAnything
		\return returns true if the mapping was successful otherwise false */
	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script);

private:
	LoggingMapper();
	LoggingMapper(const LoggingMapper &);
	LoggingMapper &operator=(const LoggingMapper &);
};

#endif
