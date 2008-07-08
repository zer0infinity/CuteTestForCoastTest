/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LoggingAction_H
#define _LoggingAction_H

//---- Action include -------------------------------------------------
#include "config_applog.h"
#include "Action.h"
#include "AppLog.h"

//---- LoggingAction ----------------------------------------------------------
//! Action which triggers logging on a channel
/*!
\par Configuration
\code
{
	/Channel		String		mandatory, channel name to log to
	/Severity		long		optional, default AppLogModule::eINFO, Severity [CRITICAL=1, FATAL=2, ERROR=4, WARN=8, INFO=16, OK=32, MAINT=64, DEBUG=128], all levels lower_equal (<=) the specified value will get logged
}
\endcode
*/
class EXPORTDECL_APPLOG LoggingAction : public Action
{
public:
	//--- constructors
	LoggingAction(const char *name);
	~LoggingAction();

	//! Logs on the Channel defined by <I>config /Channel</I>
	/*!	\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within.
		\param config the configuration of the action.
		\return true if the action run successfully, false if an error occurred. */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

};

//---- TimeLoggingAction ----------------------------------------------------------
//! Triggers logging of timing entries onto a channel
/*!
\par Configuration
\code
{
	/Channel		String		mandatory, channel name to log to
	/TimeEntries	String		optional, [Request|Method|Request], default all collected entries, path expression to lookup time entries in context with base key <Log.Times>
	/Severity		long		optional, default AppLogModule::eINFO, Severity [CRITICAL=1, FATAL=2, ERROR=4, WARN=8, INFO=16, OK=32, MAINT=64, DEBUG=128], all levels lower_equal (<=) the specified value will get logged
}
\endcode
*/
class EXPORTDECL_APPLOG TimeLoggingAction : public Action
{
public:
	//--- constructors
	TimeLoggingAction(const char *name);
	~TimeLoggingAction();

	//! Logs timing entries defined by <I>config /TimeEntries</I> on the Channel defined by <I>config /Channel</I>
	/*!	\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within.
		\param config the configuration of the action.
		\return true if the action run successfully, false if an error occurred. */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

protected:
	//! generate logentries by traversing substructures of different paths
	/*!	\param entryPath the path traversed so far
		\param entry the entry to be logged; it can contain substructures or be an array
		\param ctx the context
		\param channel the channel to log to */
	virtual bool GenLogEntries(const String &entryPath, const ROAnything &entry, Context &ctx, const String &channel, AppLogModule::eLogLevel iLevel);

	//! generate logentries for all records with the same path
	/*!	\param entryPath the path traversed so far
		\param entry the entry to be logged; it can contain substructures or be an array
		\param ctx the context
		\param channel the channel to log to */
	virtual bool GenLogEntry(const String &entryPath, const ROAnything &entry, Context &ctx, const String &channel, AppLogModule::eLogLevel iLevel);
};

#endif
