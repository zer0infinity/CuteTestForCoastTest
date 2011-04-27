/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "DataAccessStresser.h"
#include "Timers.h"
#include "StringStream.h"
#include "DataAccess.h"
RegisterStresser(DataAccessStresser);

Anything DataAccessStresser::Run(long id) {
	StartTrace(DataAccessStresser.Run);

	long nError(0);
	long num(0);
	long sum(0);
	long itopia_min(200000);
	long itopia_max(0);
	// get the DataAccess's name from configuration
	String daName = Lookup("DataAccess", "");
	TraceAny(fConfig, "Config");
	Anything results, anyRet;
	if (daName != "") {
		Anything env;
		env["Id"] = id - 1;

		// Create an empty Context and pass the stressers config into its environment store
		Context ctx(fConfig.DeepClone(), env, 0, 0, 0);
		DataAccess da(daName);
		// 10 calls as default
		num = Lookup("NumberOfCalls", 10L);
		for (long i = 1; i <= num; i++) {
			Anything anyStepResult;
			String strStepNr;
			strStepNr << i;
			DiffTimer timer;
			if (!da.StdExec(ctx)) {
				String strBuf;
				OStringStream outstream(strBuf);
				ctx.GetTmpStore()["Mapper"].PrintOn(outstream, true);
				outstream.flush();
				SystemLog::WriteToStderr(strBuf);
				nError++;
				results["ErrorMessageCtr"][strStepNr] = ctx.GetTmpStore()["Mapper"];
			}
			long accessTime = timer.Diff();
			Trace("AccessTime " << accessTime);
			sum += accessTime;
			ROAnything roaInfoMessage;
			if (Lookup("InfoMessage", roaInfoMessage) && roaInfoMessage.GetSize()) {
				TraceAny(roaInfoMessage, "slots to Output");
				Anything metaInfo = Anything(Anything::ArrayMarker());
				results["InfoMessageCtr"][strStepNr] = metaInfo;
				SlotCopier::Operate(ctx.GetTmpStore(), metaInfo, roaInfoMessage);
				TraceAny(metaInfo, "copied slots");
			}
			if (accessTime > itopia_max) {
				itopia_max = accessTime;
			}
			if (accessTime < itopia_min || itopia_min < 0) {
				itopia_min = accessTime;
			}
			anyStepResult["ExecTime"] = accessTime;
			anyStepResult["Label"] = strStepNr;
			results["Details"][strStepNr] = anyStepResult;
		}
		results["Nr"] = 1L;
		results["Steps"] = num;
		results["Sum"] = sum;
		results["Max"] = itopia_max;
		results["Min"] = itopia_min;
		results["Error"] = nError;
	}
	TraceAny(results, "Results");
	anyRet.Append(results);
	return anyRet;
}
