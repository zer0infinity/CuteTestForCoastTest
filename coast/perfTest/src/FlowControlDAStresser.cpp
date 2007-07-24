/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "FlowControlDAStresser.h"

//--- project modules used -----------------------------------------------------
#include "FlowController.h"
#include "Application.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "SysLog.h"
#include "Dbg.h"

//---- FlowControlDAStresser -----------------------------------------------------------
RegisterStresser(FlowControlDAStresser);

FlowControlDAStresser::FlowControlDAStresser(const char *FlowControlDAStresserName)
	: Stresser(FlowControlDAStresserName)
{
	StartTrace(FlowControlDAStresser.Ctor);
}

FlowControlDAStresser::~FlowControlDAStresser()
{
	StartTrace(FlowControlDAStresser.Destructor);
}

Anything FlowControlDAStresser::Run( long id)
{
	StartTrace(FlowControlDAStresser.Run);
	TraceAny(fConfig, "Config 1");
	Anything result;

	long nError(0);
	long nrBytes(0);
	long sum(0);
	long itopia_min(200000);
	long itopia_max(0);
	// get the DataAccess's name from configuration

	String flowCntrlName = fConfig["FlowController"].AsString("");
	long nrRequests = 0;
	long nrSteps = 0;

	Trace("flowcontrolname is: " << flowCntrlName);

	if (flowCntrlName != "") {
		FlowController *flowCntrl = FlowController::FindFlowController(flowCntrlName);
		if (!flowCntrl) {
			Trace("ERROR: flowcontrolname " << flowCntrlName << " not found" );
			String errorMsg = "FlowController ";
			errorMsg << flowCntrlName << " not found";
			result["ErrorMsg"][errorMsg] = "";
			return result;
		} else {
			// make unique instance name
			flowCntrlName << "_of_DAStresser";
			flowCntrl = (FlowController *)flowCntrl->ConfiguredClone("FlowController", flowCntrlName, true);
		}
		Anything env;
		env["Id"] = id - 1;

		// Create an empty Context and pass the stresser's config into its environment store
		Trace("making ctx");
		Context ctx(fConfig.DeepClone(), env, 0, 0, 0);

		// perhaps hooks for mike specific stuff here later...
		Anything tmpStore = ctx.GetTmpStore();
		tmpStore["result"].Remove("InfoMessageCtr");
		tmpStore["result"].Remove("ErrorMessage"); // init error messages...and set in ctx to be manipulated there,
		tmpStore["result"]["ConfigStep"] = 1; // sending this info down into app, for use when eMsgs are built..
		tmpStore["result"].Remove("StepsWithErrors");
		bool noBreakCondition = true;

		long Start = 0;
		long End = 0;
		long Range = 0;

		if ( fConfig.IsDefined( "Range" ) ) {
			Range = fConfig["Range"].AsLong();
			Start = (id - 1) * Range;
			End = Start + Range - 1;
		} else {
			Start = fConfig["UserList"][id-1]["Start"].AsLong();
			End = fConfig["UserList"][id-1]["End"].AsLong();
		}

		Trace("find application StressApp" );

		long currentOffset = 0;
		String appName;
		Application *application = Application::GetGlobalApplication(appName);
		if ( application ) {
			currentOffset = application->Lookup("OFFSET", 0L);
			Trace(appName << " application found" );
		}

		Start += currentOffset;
		End += currentOffset;

		long Diff = End - Start;
		long currentReqNr = 0;
		long currentNumber = 0;
		long lastErrors = 0;

		if (Diff <= 0) {
			Diff = 1;
		}

		Trace("id: [" << id << "] Range: [" << Range << "] Start: [" << Start << "] End: [" << End << "] Diff: [" << Diff  << "]");
		while ( true ) {
			Trace("PrepareRequest" );
			bool bPrepareRequestSucceeded;
			noBreakCondition = flowCntrl->PrepareRequest(ctx, bPrepareRequestSucceeded);

			// break condition check was here -----
			if ( !noBreakCondition || !bPrepareRequestSucceeded) {
				Trace("break condition-return" );
				if (!bPrepareRequestSucceeded) {
					nError++;
					Trace("Errorcount1:" << nError );
				}
				if (flowCntrl) {
					flowCntrl->Finalize();
					delete flowCntrl;
				}
				if (nrSteps == 0) {
					nrSteps++;
				}
				break;
			}
			nrSteps++;
			// stepnumber as string for use within this loop
			String strStepNr;
			strStepNr << nrSteps;

			TraceAny( ctx.GetQuery(), "state of query after PrepareRequest is" );
			currentReqNr = tmpStore["FlowState"]["RunNr"].AsLong();
			currentNumber = ( currentReqNr % Diff ) + Start;
			tmpStore["CurrentNumber"] = currentNumber;

			// check if PrepareRequest succeeded, if it failed, set an Error and proceed with the next test
			if (!bPrepareRequestSucceeded) {
				// check if we wanted PrepareRequest to fail
				ROAnything roa;
				if (ctx.Lookup("PrepareRequestFail", roa) && roa.AsBool(false)) {
				} else {
					nError++;
					Trace("Errorcount2:" << nError );
				}
			} else {
				// prepare DataAccess, possible to overwrite default of config with slot in query
				if (!ctx.GetQuery().IsDefined("DataAccess")) {
					// use default DataAccess name from general config
					ctx.GetQuery()["DataAccess"] = fConfig["DataAccess"].AsString("");
				}

				TraceAny(ctx.GetTmpStore(), "Tempstore");

				// connect to server and place request and extract reply...
				long accessTime;
				if (!flowCntrl->ExecDataAccess(ctx, accessTime)) {
					Trace("ExecDataAccess failed!");
					// check if we wanted da.StdExec(ctx) to fail
					ROAnything roa;
					if (ctx.Lookup("StdExecFail", roa) && roa.AsBool(false)) {
					} else {
						nError++;
						Trace("Errorcount3:" << nError );
					}
				} else {
					Trace("AccessTime " << accessTime);
					if ( !flowCntrl->AnalyseReply(ctx, result) ) {
						Trace("Analyse reply failed!");
						// check if we wanted AnalyseReply to fail
						ROAnything roa;
						if (ctx.Lookup("AnalyseReplyFail", roa) && roa.AsBool(false)) {
						} else {
							nError++;
							Trace("AnalyseReply returned false");
							Trace("Errorcount4:" << nError );
						}
					}
					if ( tmpStore["result"].IsDefined("Bytes" ) ) {
						nrBytes += tmpStore["result"]["Bytes"].AsLong(0L);
						Trace("Total no bytes is: " << nrBytes);
						tmpStore["result"].Remove("Bytes");
					}
				}
				nrRequests++;

				TraceAny(ctx.GetTmpStore(), "outComing Tempstore");

				tmpStore["result"]["Details"][strStepNr]["ExecTime"] = accessTime;
				sum += accessTime;
				if (accessTime > itopia_max) {
					itopia_max = accessTime;
				}
				if (accessTime < itopia_min || itopia_min < 0) {
					itopia_min = accessTime;
				}
			}

			if (tmpStore.IsDefined("Label")) {
				tmpStore["result"]["Details"][strStepNr]["Label"] = tmpStore["Label"].AsString();
			} else {
				tmpStore["result"]["Details"][strStepNr]["Label"] = tmpStore["result"]["ConfigStep"].AsString();
			}

			// if there were errors fill them into a separate Any
			CheckCopyErrorMessage(result, ctx.GetTmpStore(), nrSteps, (lastErrors != nError));
			lastErrors = nError;
			tmpStore["result"]["ConfigStep"] = nrSteps + 1;
			SysLog::WriteToStderr(".", 1);	// progress indication
			// remove slots which should not stay persistent between requests
			// enable FlowController to cleanup its own mess
			flowCntrl->CleanupAfterStep(ctx);
		}

		if ( tmpStore["result"].IsDefined("InfoMessageCtr" ) ) { // info (trace) msgs
			result["InfoMessageCtr"] = tmpStore["result"]["InfoMessageCtr"];
		}

		tmpStore["result"].Remove("StepsWithErrors");
		TraceAny(tmpStore["result"], "temp store" );
		result["Details"] = tmpStore["result"]["Details"];
	}

	result["Nr"] = nrRequests;
	result["Steps"] = nrSteps;
	result["Error"] = nError;
	result["Sum"] = sum;
	result["Max"] = itopia_max;
	result["Min"] = itopia_min;
	result["Bytes"] = nrBytes;

	TraceAny(result, "Result (transactions include relocate/refresh)" );
	Anything anyResult;
	anyResult.Append(result);
	return anyResult;
}
