/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "HTMLComparer.h"
#include "DataAccess.h"
#include "Context.h"
#include "Tracer.h"

//--- interface include --------------------------------------------------------
#include "ATTFlowController.h"

//---- ATTFlowController ----------------------------------------------------------------
RegisterFlowController(ATTFlowController);

ATTFlowController::ATTFlowController(const char *FlowControllerName)
	: HttpFlowController(FlowControllerName), fError(false)
{
	StartTrace(ATTFlowController.Ctor);
}

ATTFlowController::~ATTFlowController()
{
	StartTrace(ATTFlowController.Dtor);
}

bool ATTFlowController::ExecDataAccess(Context &ctx, long &execTime)
{
	StartTrace(ATTFlowController.ExecDataAccess);

	bool retVal = HttpFlowController::ExecDataAccess(ctx, execTime);
	if (retVal) {
		retVal = GetExpected(ctx);
		if (retVal) {
			return retVal;
		}
	}
	// on error we come to here
	fError = true;
	Anything tmpStore = ctx.GetTmpStore();
	tmpStore["result"]["ErrorMessage"].Append(tmpStore["Error"]);

	return retVal;
}

bool ATTFlowController::GetExpected(Context &ctx)
{
	StartTrace(ATTFlowController.GetExpected);

	bool boRet = false;
	String daName("GetExpected");
	ctx.Lookup("ControlDataAccess", daName);
	Trace("using DA: " << daName << " to get the control file");
	ctx.GetTmpStore().Remove(daName);
	DataAccess da(daName);
	boRet = da.StdExec(ctx);
	TraceAny(ctx.GetTmpStore(), "TempStore after");
	return boRet;

}

bool ATTFlowController::AnalyseReply(Context &ctx, Anything &result)
{
	StartTrace(ATTFlowController.AnalyseReply);

	bool boRet = DoCompareHTMLDocs(ctx, result);

	return boRet & HttpFlowController::AnalyseReply(ctx, result);
}

bool ATTFlowController::DoCompareHTMLDocs(Context &ctx, Anything &result)
{
	StartTrace(ATTFlowController.DoCompareHTMLDocs);

	Anything sample = ctx.GetTmpStore()["Mapper"]["HTTPBody"];
	Anything expected = ctx.GetTmpStore()["GetExpected"]["HTTPBody"];

	HTMLComparer comparer(expected, sample);

	String report;
	bool boRet = comparer.Compare(report);
	if (!boRet) {
		report << " // In Request Nr. " << ctx.GetTmpStore()["FlowState"]["RequestNr"].AsString("x");
		Trace("Compare failed: " << report);
		result["CompareResults"] = report;
		fError = true;
	} else {
		Trace("Compare successful");
	}

	return boRet;

}

bool ATTFlowController::PrepareRequest(Context &ctx, bool &bPrepareRequestSucceeded)
{
	StartTrace(ATTFlowController.PrepareRequest);

	if (fError) {
		Trace("In Error State");
		return false;
	}
	return HttpFlowController::PrepareRequest(ctx, bPrepareRequestSucceeded);
}

//bool ATTFlowController::PrepareRequest(Context &ctx) {
//  StartTrace(ATTFlowController.PrepareRequest);
//
//  if(fError) {
//	return false;
//  }
//  return HttpFlowController::PrepareRequest(ctx);
//}

