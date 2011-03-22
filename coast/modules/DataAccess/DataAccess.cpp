/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DataAccess.h"
#include "Timers.h"

//---- DataAccess ----------------------------------------------------------------------
DataAccess::DataAccess(const char *trxName)
	: fName(trxName)
{
	StartTrace1(DataAccess.DataAccess, "trxName: [" << trxName << "]");
}

DataAccess::~DataAccess()
{
}

DataAccessImpl *DataAccess::GetImpl(const char *trxName, Context &context)
{
	StartTrace(DataAccess.GetImpl);
	Trace("Trx name is [" << trxName << "]");
	Assert(trxName); // precondition
	DataAccessImpl *trxImpl = DataAccessImpl::FindDataAccessImpl(trxName);
	// handling error or misconfiguration, keep on same line for clearness reason of output message
	Assert(trxImpl);
	if (!trxImpl) {
		HandleError(context, trxName, __FILE__, __LINE__, "DataAccess::GetImpl returned 0");
	}
	return trxImpl;
}

bool DataAccess::StdExec(Context &trxContext)
{
	StartTrace(DataAccess.StdExec);
	DAAccessTimer(DataAccess.StdExec, fName, trxContext);

	DataAccessImpl *trx = GetImpl(fName, trxContext);
	bool result = false;

	if (trx) {
		trxContext.Push("DataAccess", trx);
		ParameterMapper *params = 0;
		bool pIsTemp = GetMyParameterMapper(trxContext, params);
		ResultMapper *results = 0;
		bool rIsTemp = GetMyResultMapper(trxContext, results);

		// execute data access
		result = Exec(params, results, trxContext);

		// cleanup mappers that were created on the fly
		// as interpreters for scripting
		if (pIsTemp) {
			delete params;
		}
		if (rIsTemp) {
			delete results;
		}
		trxContext.Remove("DataAccess");
	}
	return result;
}

bool DataAccess::Exec(ParameterMapper *params, ResultMapper *results, Context &trxContext)
{
	StartTrace(DataAccess.Exec);
	DAAccessTimer(DataAccess.Exec, fName, trxContext);

	// if we don't have a complete triple, return immediately
	DataAccessImpl *trx = GetImpl(fName, trxContext);
	Assert(trx && params && results);
	if ( !(trx && params && results) ) {
		return false;
	}

	//SOP: check if we can speed up responsiveness by unlocking the session during IO
	SessionReleaser slr(trxContext);
	slr.Use();

	// execute the TRX (both mappers should be defined now)
	bool ret = trx->Exec(trxContext, params, results);

	// return result
	return ret;
}

bool DataAccess::GetMyParameterMapper(Context &c, ParameterMapper *&pm)
{
	StartTrace(DataAccess.GetMyParameterMapper);
	bool isScriptInterpreter;

	// look into own config
	ROAnything script = c.Lookup("ParameterMapperScript");
	if (script.GetType() == AnyArrayType) {
		isScriptInterpreter = true;

		Trace("Parameter script found. Using interpreter.");
		pm = new (Coast::Storage::Current()) EagerParameterMapper(String("ParamScriptInterpreterFor") << fName, script);
	} else {
		// no script present, check if a named mapper can be found
		isScriptInterpreter = false;

		String name(script.AsString(fName));
		pm = ParameterMapper::FindParameterMapper(name);
		if (pm) {
			Trace("Using specified ParameterMapper: " << name);
		} else {
			// is there a fallback mapper defined?
			String fallback = c.Lookup("FallbackParameterMapper", "");

			pm = ParameterMapper::FindParameterMapper(fallback);
			if (pm) {
				Trace("Using fallback ParameterMapper: " << fallback);
			} else {
				Trace("ERROR: No mapper with name [" << name << "] found.");
				HandleError(c, name, __FILE__, __LINE__, "DataAccess::GetMyParameterMapper returned 0");
			}
		}
	}
	return isScriptInterpreter;
}

bool DataAccess::GetMyResultMapper(Context &c, ResultMapper *&rm)
{
	StartTrace(DataAccess.GetMyResultMapper);
	bool isScriptInterpreter;

	// look into own config
	ROAnything script = c.Lookup("ResultMapperScript");
	if (script.GetType() == AnyArrayType) {
		isScriptInterpreter = true;

		Trace("Result script found. Using interpreter.");
		rm = new (Coast::Storage::Current()) EagerResultMapper(String("ResultScriptInterpreterFor") << fName, script);
	} else {
		// no script present, check if a named mapper can be found
		isScriptInterpreter = false;

		String name(script.AsString(fName));
		rm = ResultMapper::FindResultMapper(name);
		if (rm) {
			Trace("Using specified ResultMapper: " << name);
		} else {
			// is there a fallback mapper defined?
			String fallback = c.Lookup("FallbackResultMapper", "");

			rm = ResultMapper::FindResultMapper(fallback);
			if (rm) {
				Trace("Using fallback ResultMapper: " << fallback);
			} else {
				Trace("ERROR: No mapper with name [" << name << "] found.");
				HandleError(c, name, __FILE__, __LINE__, "DataAccess::GetMyResultMapper returned 0");
			}
		}
	}
	return isScriptInterpreter;
}

void DataAccess::HandleError(Context &context, String mapperName, const char *file, long line, String msg)
{
	StartTrace(DataAccess.HandleError);
	// cut off path in file string (only output file)
	String filePath(file);
	long pos = filePath.StrRChr('\\');			// windows path?
	if (pos < 0) {
		pos = filePath.StrRChr('/');    // unix path?
	}

	String logMsg = (pos < 0) ? filePath : filePath.SubString(pos + 1);
	logMsg << ":" << line << " " << msg << " for [" << mapperName << "]";
	SystemLog::Error(logMsg);
	Trace(logMsg);
	context.GetTmpStore()["DataAccess"][mapperName]["Error"].Append(logMsg);
}
