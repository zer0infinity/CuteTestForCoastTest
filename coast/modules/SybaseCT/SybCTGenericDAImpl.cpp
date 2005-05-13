/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTGenericDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Dbg.h"

//--- SybCTGenericDAImpl -----------------------------------------------------
RegisterDataAccessImpl(SybCTGenericDAImpl);

SybCTGenericDAImpl::SybCTGenericDAImpl(const char *name)
	: DataAccessImpl(name)
{
}

SybCTGenericDAImpl::~SybCTGenericDAImpl()
{
}

IFAObject *SybCTGenericDAImpl::Clone() const
{
	return new SybCTGenericDAImpl(fName);
}

bool SybCTGenericDAImpl::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(SybCTGenericDAImpl.Exec);
	return false;
}

void SybCTGenericDAImpl::FillParameters(Context &context, ParameterMapper *in, ResultMapper *out, Anything &queryParams )
{
	StartTrace(SybCTGenericDAImpl.FillParameters);
	String user, passwd, server, app, command, resultformat, resultsize;
	long lResultSize = 0L;
	in->Get( "SybDBUser", user, context);
	in->Get( "SybDBPW", passwd, context);
	in->Get( "SybDBHost", server, context);
	in->Get( "SybDBApp", app, context);
	in->Get( "SybDBResultFormat", resultformat, context);
	in->Get( "SybDBMaxResultSize", resultsize, context);
	lResultSize = resultsize.AsLong(0L);

	OStringStream os(command);
	in->Get("SQL", os, context);
	os.flush();
	SubTrace (Query, "QUERY IS:" << command );

	Trace ("USER IS:" << user );
	Trace ("PASSWD IS:" << passwd );
	Trace ("Host is:" << server );
	Trace ("App is:" << app );
	Trace ("ResultFormat is:" << resultformat );
	Trace ("ResultSize is:" << lResultSize << " kb" );
	out->Put("Query", command, context);
	queryParams["user"] = user;
	queryParams["password"] = passwd;
	queryParams["server"] = server;
	queryParams["app"] = app;
	queryParams["query"] = command;
	queryParams["resultformat"] = resultformat;
	queryParams["resultsize"] = lResultSize;
}

bool SybCTGenericDAImpl::PutResults(Context &context, ParameterMapper *in, ResultMapper *out, Anything &queryParams, Anything &queryResults, Anything &queryTitles)
{
	StartTrace(SybCTGenericDAImpl.PutResults);
	bool bShowRowCount = true;
	in->Get("NoQueryCount", bShowRowCount, context);
	Trace("bShowRowCount is " << (bShowRowCount ? "true" : "false"));
	if ( queryResults.IsDefined("SP_Retcode") ) {
		Trace("StoredProcedure return code: " << queryResults["SP_Retcode"].AsString());
		out->Put("SP_Retcode", queryResults["SP_Retcode"], context);
		queryResults.Remove("SP_Retcode");
	}
	if (bShowRowCount) {
		out->Put("QueryCount", queryResults.GetSize(), context);
	}
	if (queryParams["resultformat"].AsString().Compare("TitlesOnce") == 0) {
		TraceAny(queryTitles, "titles once was defined and contains");
		out->Put("QueryTitles", queryTitles, context);
	}
	out->Put("QueryResult", queryResults, context);
	out->Put("QuerySource", queryParams["server"].AsString(), context);
	return true;
}

void SybCTGenericDAImpl::PutMessages(Context &context, ResultMapper *out, Anything &aMsgAny)
{
	StartTrace(SybCTGenericDAImpl.PutMessages);
	if ( aMsgAny.IsDefined("Messages") ) {
		TraceAny(aMsgAny, "SybaseCT Messages");
		out->Put("Messages", aMsgAny["Messages"], context);
		if ( aMsgAny.IsDefined( "MainMsgErr" ) ) {
			String mainMsgErr;
			GetName(mainMsgErr);
			Trace("myName is [" << mainMsgErr << "]");
			mainMsgErr << ": " << aMsgAny["MainMsgErr"].AsString("");
			out->Put("MainMsgErr", mainMsgErr, context);
			if ( aMsgAny.IsDefined( "MainMsgErrNumber" ) ) {
				out->Put("MainMsgErrNumber", aMsgAny["MainMsgErrNumber"].AsString(""), context);
			} else {
				out->Put("MainMsgErrNumber", String("34000"), context);
			}
		}
	}
}
