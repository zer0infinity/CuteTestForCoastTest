/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Threads.h"
#include "Mapper.h"
#include "StringStream.h"
#include "Context.h"
#include "SybDB.h"
#include "Renderer.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "SybDBDAImpl.h"

//---- SybDBDAImpl ----------------------------------------------------------------
RegisterDataAccessImpl(SybDBDAImpl);

SybDBDAImpl::SybDBDAImpl(const char *name) : DataAccessImpl(name)
{
}

SybDBDAImpl::~SybDBDAImpl()
{
}

IFAObject *SybDBDAImpl::Clone() const
{
	return new SybDBDAImpl(fName);
}

bool SybDBDAImpl::Exec( Context &context, InputMapper *in, OutputMapper *out)
{
	StartTrace(SybDBDAImpl.Exec);

	SybDB aDB;
	bool retCode = false;

	if ( aDB.Init() ) {
		Anything anyuser, anypasswd;
		String user, passwd, server, app, command;
		in->Get( "SybDBUser", anyuser, context);
		in->Get( "SybDBPW", anypasswd, context);
		in->Get( "SybDBHost", server, context);
		in->Get( "SybDBApp", app, context);

		OStringStream os(command);
		in->Get("SQL", os, context);
		os.flush();
		SubTrace (Query, "QUERY IS:" << command );

		Renderer::RenderOnString(user, context, anyuser);
		Renderer::RenderOnString(passwd, context, anypasswd);

		Trace ("USER IS:" << user );
		Trace ("PASSWD IS:" << passwd );
		out->Put("Query", command, context);

		if ( !aDB.Open( user, passwd, server, app ) ) {
			Trace( "SybDBDAImpl::Exec could not open SyBase");
		} else {
			if ( !aDB.SqlExec( command ) ) {
				Trace( "SybDBDAImpl::Exec could not execute the sql command");
			} else {
				Anything queryResults;
				if ( aDB.GetResult( queryResults ) ) {
					TraceAny( queryResults, "computed result = " );
					bool bShowRowCount;
					in->Get("NoQueryCount", bShowRowCount, context);
					if (bShowRowCount) {
						out->Put("QueryCount", queryResults.GetSize(), context);
					}
					out->Put("QueryResult", queryResults, context);
					retCode = true;
				} else {
					Trace( "SybDBDAImpl::Exec could not fetch the result");
				}
			}
			aDB.Close();
		}
	}
	{
		SimpleMutexEntry aME(SybDB::fgMessagesMutex);
		aME.Use();
		const Anything &aMsgAny = SybDB::fgMessages;
		if ( aMsgAny.IsDefined("Messages") ) {
			TraceAny(aMsgAny, "SybDBMessages");
			out->Put("Messages", Anything(context.Lookup("sessionId", "noSession")), context);
			out->Put("Messages", aMsgAny["Messages"], context);
			if ( aMsgAny.IsDefined( "MainMsgErr" ) ) {
				String mainMsgErr;
				GetName(mainMsgErr);
				mainMsgErr << ": " << aMsgAny["MainMsgErr"].AsString("");
				out->Put("MainMsgErr", mainMsgErr, context);
			}
			SybDB::fgMessages.Remove("Messages");
			SybDB::fgMessages.Remove("MainMsgErr");
			TraceAny(SybDB::fgMessages, "SybDB::fgMessages after Remove");
		}
	}
	Trace("SybDBDAImpl::Exec returning " << (retCode ? "true" : "false"));
	return retCode;
}
