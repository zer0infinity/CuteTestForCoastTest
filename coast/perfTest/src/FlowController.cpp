/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "FlowController.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "Application.h"
#include "DataAccess.h"
#include "Timers.h"
#include "Dbg.h"

RegisterFlowController(FlowController);
//---- FlowController -----------------------------------------------------------
FlowController::FlowController(const char *FlowControllerName) : ConfNamedObject(FlowControllerName)
{
	StartTrace1(FlowController.FlowController, "<" << FlowControllerName << ">");
}

FlowController::~FlowController()
{
}

//---- registry interface
RegCacheImpl(FlowController);	// FindFlowController()

bool FlowController::PrepareRequest(Context &ctx)
{
	StartTrace(FlowController.PrepareRequest);

	long reqNr = GetRequestNr(ctx);

	// number of runs of pre depends on number of slots in anything
	long nrOfPreRunRequests = fConfig["PreRun"].GetSize();

	Anything tmpStore = ctx.GetTmpStore();
	Anything flowState = tmpStore["FlowState"];
	if (!flowState["PreRunDone"].AsBool(1)) {
		Trace("PRE RUN NOT DONE ");

		if (reqNr < nrOfPreRunRequests) {
			DoPrepare(tmpStore, fConfig["PreRun"][reqNr]);
			flowState["RequestNr"] = ++reqNr;
			TraceAny(tmpStore["FlowState"], "Flow State on exit");
			return true;
		} else {
			// PreRun done , reset for normal run
			reqNr = 0;
			flowState["RunNr"] = 0L;
			flowState["PreRunDone"] = true;
		}
	}

	long nrOfRuns = 0;
	String appName;
	Application *application = Application::GetGlobalApplication(appName);
	if ( application ) {
		nrOfRuns = application->Lookup("NumberOfRuns", fConfig["NumberOfRuns"].AsLong(1));
		Trace(appName << " application found" );
	} else {
		nrOfRuns = fConfig["NumberOfRuns"].AsLong(1);
	}

	long runNr = flowState["RunNr"].AsLong(0);
	Trace("INIT Number of Run: " << runNr <<  " of " << nrOfRuns);

	while (runNr < nrOfRuns) { // loop thru steps and incr. runNr after each batch has been processed
		Trace("Number of Run: " << runNr <<  " of " << nrOfRuns);
		TraceAny(fConfig["Run"], "Config Run");
		long nrOfRequests = fConfig["Run"].GetSize();
		if (reqNr < nrOfRequests) {
			DoPrepare(tmpStore, fConfig["Run"][reqNr]);
			flowState["RequestNr"] = ++reqNr;
			TraceAny(tmpStore, "tmpStore on exit");
			TraceAny(tmpStore["FlowState"], "Flow State on exit");
			return true;
		}
		reqNr = 0;	// reset request number
		flowState["RunNr"] = ++runNr;
	}

	TraceAny(tmpStore["FlowState"], "Flow State on exit with false");
	return false;
}
ROAnything FlowController::GetStepConfig(Context &ctx)
{
	StartTrace(FlowController.GetStepConfig);

	return fConfig["Run"][GetRequestNr(ctx)];
}

long FlowController::GetRequestNr(Context &ctx)
{
	StartTrace(FlowController.GetRequestNr);
	// retrieve state from tempStore -- this is done to be thread safe
	// State contains
	// {
	//	/PreRunDone		True if prerun has run
	//	/RunNr			Number of the actual run
	//	/RequestNr		Number of the next request
	// }
	Anything tmpStore = ctx.GetTmpStore();

	// default built in tmpStore if no tmpStore built by previous step
	if (!tmpStore.IsDefined("FlowState")) {
		tmpStore["FlowState"]["RequestNr"] = 0L;
		tmpStore["FlowState"]["PreRunDone"] = false;
		tmpStore["FlowState"]["RunNr"] = 0L;
		//TraceAny(tmpStore["FlowState"],"Flow State initialized");
	}
	Anything flowState = tmpStore["FlowState"];

	// Check PreRun
	return flowState["RequestNr"].AsLong(0);

}

bool FlowController::PrepareRequest(Context &ctx, bool &bPrepareRequestSucceeded)
{
	// default implementation, can be overridden in a subclass
	// this method allows to also return a code when the preparation of the request
	//  failed and not only when all requests are done at the end
	bPrepareRequestSucceeded = true;
	return PrepareRequest(ctx);
}

void FlowController::DoPrepare(Anything &dest, const ROAnything &runConfig)
{
	StartTrace(FlowController.DoPrepare);

	// copies Config step to tmpStore for the "current step"
	long sz = runConfig.GetSize();
	for (long i = 0; i < sz; i++) {
		String slotName = runConfig.SlotName(i);
		if ( slotName ) {
			dest[slotName] = runConfig[slotName].DeepClone();
		}
	}
}

bool FlowController::ExecDataAccess(Context &ctx, long &execTime)
{
	StartTrace(FlowController.ExecDataAccess);
	execTime = 0;
	bool boRet = false;
	if (ctx.GetQuery().IsDefined("DataAccess")) {
		String daName = ctx.GetQuery()["DataAccess"].AsString();
		Trace("using DA: " << daName);
		DataAccess da(daName);
		DiffTimer timer;
		boRet = da.StdExec(ctx);
		execTime = timer.Diff();
	}
	return boRet;
}

void FlowController::Init(Context &ctx)
{
	StartTrace(FlowController.Init);
	// dummy implementation
	return;
}

bool FlowController::AnalyseReply(Context &ctx)
{
	StartTrace(FlowController.AnalyseReply);
	// dummy implementation
	return true;
}

bool FlowController::AnalyseReply(Context &ctx, Anything &result)
{
	StartTrace(FlowController.AnalyseReply);
	// dummy implementation to behave as without passing the result any
	return AnalyseReply(ctx);
}

void FlowController::CleanupAfterStep(Context &ctx)
{
	StartTrace(FlowController.CleanupAfterStep);
	ROAnything roaStepConfig = ROAnything(fConfig)["Run"][GetRequestNr(ctx)-1L];
	DoCleanupAfterStep(ctx, roaStepConfig);
}

void FlowController::DoCleanupAfterStep(Context &ctx, ROAnything roaStepConfig)
{
	StartTrace(FlowController.DoCleanupAfterStep);
	Anything tmpStore(ctx.GetTmpStore());
	SubTraceAny(TmpStore, ctx.GetTmpStore(), "TmpStore before");
	if ( !roaStepConfig.IsNull() ) {
		long sz = roaStepConfig.GetSize();
		for (long i = 0; i < sz; i++) {
			String slotName = roaStepConfig.SlotName(i);
			if ( slotName ) {
				Trace("Slotname to remove : " << slotName);
				tmpStore.Remove(slotName);
			}
		}
	}
	SubTraceAny(TmpStore, tmpStore, "TmpStore after");
}

//---- FlowControllersModule -----------------------------------------------------------
RegisterModule(FlowControllersModule);

FlowControllersModule::FlowControllersModule(const char *name) : WDModule(name)
{
}

FlowControllersModule::~FlowControllersModule()
{
}

bool FlowControllersModule::Init(const Anything &config)
{
	StartTrace(FlowControllersModule.Init);
	TraceAny(config["FlowControllers"], " Config ");
	if (config.IsDefined("FlowControllers")) {
		AliasInstaller ai("FlowController");
		return RegisterableObject::Install(config["FlowControllers"], "FlowController", &ai);
	}
	return false;
}

bool FlowControllersModule::Finis()
{
	return StdFinis("FlowController", "FlowControllers");
}
