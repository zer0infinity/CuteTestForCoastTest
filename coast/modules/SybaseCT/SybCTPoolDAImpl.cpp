/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTPoolDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "SybaseModule.h"
#include "SybCTPoolManager.h"
#include "DiffTimer.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- SybCTPoolDAImpl ----------------------------------------------------------------
RegisterDataAccessImpl(SybCTPoolDAImpl);

SybCTPoolDAImpl::SybCTPoolDAImpl(const char *name)
	: SybCTGenericDAImpl(name)
{
	StartTrace(SybCTPoolDAImpl.SybCTPoolDAImpl);
}

SybCTPoolDAImpl::~SybCTPoolDAImpl()
{
	StartTrace(SybCTPoolDAImpl.~SybCTPoolDAImpl);
}

IFAObject *SybCTPoolDAImpl::Clone() const
{
	StartTrace(SybCTPoolDAImpl.Clone);
	return new SybCTPoolDAImpl(fName);
}

bool SybCTPoolDAImpl::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(SybCTPoolDAImpl.Exec);

	bool retCode = false;
	Anything aMsgAny, queryParams;
	SybCTPoolManager *pPool = NULL;

	FillParameters(context, in, out, queryParams);
	if (queryParams["server"].AsString().Length()) {
		SybaseModule *pModule = (SybaseModule *)WDModule::FindWDModule("SybaseModule");
		if (pModule != NULL) {
			pPool = pModule->GetPoolManager(queryParams["server"].AsString());
		}
	}

	if (pPool) {
		Anything workerConfig, queryResults, queryTitles;

		// create Anything with parameters to pass
		workerConfig = queryParams;
		workerConfig["messages"] = (IFAObject *)&aMsgAny;
		workerConfig["results"] = (IFAObject *)&queryResults;
		workerConfig["titles"] = (IFAObject *)&queryTitles;

		// this call blocks until the worker has finished working
		{
			Trace("starting worker");
			DiffTimer aTimer;
			pPool->Work(workerConfig);
			Trace("sql used: " << aTimer.Diff() << "ms");
		}

		// check the result
		if (aMsgAny["RetCode"].AsBool() == true) {
			// remove the RetCode slot
			aMsgAny.Remove("RetCode");
			// now process the results
			SubTraceAny(ResultAny, queryTitles, "computed Titles = " );
			SubTraceAny(ResultAny, queryResults, "computed result = " );
			retCode = PutResults(context, in, out, queryParams, queryResults, queryTitles);
		}
	} else {
		// no Pool found for server, create appropriate message
		Anything aMsg;
		aMsg["severity"] = 15;
		aMsg["msgtext"] = String("No PoolManager found for server '") << queryParams["server"].AsCharPtr() << "'.";
		aMsg["servername"] = queryParams["server"];
		aMsgAny["Messages"].Append(aMsg);
		aMsgAny["MainMsgErr"] = aMsg["msgtext"].AsString();
	}
	PutMessages(context, out, aMsgAny);
	Trace("SybCTPoolDAImpl::Exec returning " << (retCode ? "true" : "false"));
	return retCode;
}
