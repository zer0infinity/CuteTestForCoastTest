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

//---- LoggingAction ----------------------------------------------------------
//! triggers logging on a channel
//!	Structure of config:
//!<PRE>	{
//!		/Channel	String		# Channel to log to
//!	}</PRE>
class EXPORTDECL_APPLOG LoggingAction : public Action
{
public:
	//--- constructors
	LoggingAction(const char *name);
	~LoggingAction();

	//!Logs on the Channel defined by <I>config /Channel</I>
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

};

//---- TimeLoggingAction ----------------------------------------------------------
//! triggers logging on a channel
//!	Structure of config:
//!<PRE>	{
//!		/Channel	String		# Channel to log to
//!		/TimeEntries	<path>  # Path expression for time entries e.g. Log.Times.Method
//!	}</PRE>
class EXPORTDECL_APPLOG TimeLoggingAction : public Action
{
public:
	//--- constructors
	TimeLoggingAction(const char *name);
	~TimeLoggingAction();

	//!Logs timing entries defiend by <I>config /TimeEntries</I> on the Channel defined by <I>config /Channel</I>
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

protected:
	//!generate logentries by traversing substructures of different paths
	//! \param entryPath the path traversed so far
	//! \param entry the entry to be logged; it can contain substructures or be an array
	//! \param ctx the context
	//! \param channel the channel to log to
	virtual bool GenLogEntries(const String &entryPath, const ROAnything &entry, Context &ctx, const String &channel);

	//!generate logentries for all records with the same path
	//! \param entryPath the path traversed so far
	//! \param entry the entry to be logged; it can contain substructures or be an array
	//! \param ctx the context
	//! \param channel the channel to log to
	virtual bool GenLogEntry(const String &entryPath, const ROAnything &entry, Context &ctx, const String &channel);

};

#endif
