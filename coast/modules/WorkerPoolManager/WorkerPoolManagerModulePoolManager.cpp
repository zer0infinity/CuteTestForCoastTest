/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "WorkerPoolManagerModulePoolManager.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- WorkerPoolManagerModuleWorker -----------------------------------------------
WorkerPoolManagerModuleWorker::WorkerPoolManagerModuleWorker(const char *name)
	: WorkerThread(name)
{
	StartTrace(WorkerPoolManagerModuleWorker.WorkerPoolManagerModuleWorker);
}

WorkerPoolManagerModuleWorker::~WorkerPoolManagerModuleWorker()
{
	StartTrace(WorkerPoolManagerModuleWorker.~WorkerPoolManagerModuleWorker);

}

// one time initialization
void WorkerPoolManagerModuleWorker::DoInit(ROAnything workerInit)
{
	StartTrace(WorkerPoolManagerModuleWorker.DoInit);
	fWorkerInitCfg = workerInit.DeepClone();
}

void WorkerPoolManagerModuleWorker::DoProcessWorkload()
{
	StartTrace(WorkerPoolManagerModuleWorker.DoProcessWorkload);
	if (IsWorking() && GetState() == Thread::eRunning) {
		Anything *pMessages;
		Mutex *pMx;
		Mutex::ConditionType *pCond;
		pMx = (Mutex *)fWork["mutex"].AsIFAObject();
		pCond = (Mutex::ConditionType *)fWork["condition"].AsIFAObject();
		pMessages = (Anything *)fWork["messages"].AsIFAObject(0L);
		Assert(pMessages != NULL);
		Assert(pMx != NULL);
		Assert(pCond != NULL);
		TraceAny(*pMessages, "Arguments passed to me");
		TraceAny(fWorkerInitCfg, "My initial config:");
		{
			// here we should do something fancier then copying input to output :-)
			LockUnlockEntry me(*pMx);
			Anything *pResult = (Anything *)fWork["results"].AsIFAObject(0L);
			(*pResult)["Got"] = (*pMessages).AsString();
			(*pResult)["WorkerInitialConfig"] = fWorkerInitCfg;
		}
		// signal that we finished working
		pCond->Signal();
	}
}

// passing of the work
void WorkerPoolManagerModuleWorker::DoWorkingHook(ROAnything workerConfig)
{
	StartTrace(WorkerPoolManagerModuleWorker.DoWorkingHook);
	// It is essential to copy the worker's config into a instance variable.
	// And it is essential to DeepClone that data for thread safety reasons too.
	fWork = workerConfig.DeepClone();
}

void WorkerPoolManagerModuleWorker::DoTerminationRequestHook(ROAnything)
{
	StartTrace(WorkerPoolManagerModuleWorker.DoTerminationRequestHook);
}

void WorkerPoolManagerModuleWorker::DoTerminatedHook()
{
	StartTrace(WorkerPoolManagerModuleWorker.DoTerminatedHook);
}

//---- WorkerPoolManagerModulePoolManager ------------------------------------------------

WorkerPoolManagerModulePoolManager::WorkerPoolManagerModulePoolManager(String name)
	: WorkerPoolManager(name)
	, fRequests(0)
{
	StartTrace(WorkerPoolManagerModulePoolManager.WorkerPoolManagerModulePoolManager);
}

WorkerPoolManagerModulePoolManager::~WorkerPoolManagerModulePoolManager()
{
	StartTrace(WorkerPoolManagerModulePoolManager.~WorkerPoolManagerModulePoolManager);
	Terminate(5L);
	Anything dummy;
	DoDeletePool(dummy);
}

bool WorkerPoolManagerModulePoolManager::Init(const ROAnything config)
{
	StartTrace(WorkerPoolManagerModulePoolManager.Init);

	bool bRet = false;
	long concurrentQueries = 5L;
	String strInterfacesPathName;
	if (config.IsDefined("Workers")) {
		concurrentQueries = config["Workers"].AsLong(5L);
	}
	long usePoolStorage = 0L;
	u_long poolStorageSize = 1000L;
	u_long numOfPoolBucketSizes = 20L;
	Trace("Initializing worker pool with " << concurrentQueries << " possible parallel queries");

	Anything workerInitCfg;
	workerInitCfg = config["WorkerInitialConfig"].DeepClone();
	TraceAny(workerInitCfg, "WorkerInitialConfig for Pool: "  << workerInitCfg.SlotName(0L));
	// initialize the base class
	if (WorkerPoolManager::Init(concurrentQueries, usePoolStorage, poolStorageSize, numOfPoolBucketSizes, workerInitCfg) == 0) {
		Trace("WorkerPoolManager::Init successful");
		bRet = true;
	}
	return bRet;
}

void WorkerPoolManagerModulePoolManager::DoAllocPool(ROAnything args)
{
	StartTrace(WorkerPoolManagerModulePoolManager.DoAllocPool);
	// create the pool of worker threads
	fRequests = new WorkerPoolManagerModuleWorker[fPoolSize];
}

WorkerThread *WorkerPoolManagerModulePoolManager::DoGetWorker(long i)
{
	StartTrace(WorkerPoolManagerModulePoolManager.DoGetWorker);
	// accessor for one specific worker
	if (fRequests) {
		return &(fRequests[i]);
	}
	return 0;
}

void WorkerPoolManagerModulePoolManager::DoDeletePool(ROAnything args)
{
	StartTrace(WorkerPoolManagerModulePoolManager.DoDeletePool);
	// cleanup of the sub-class specific stuff
	// CAUTION: this cleanup method may be called repeatedly..

	if (fRequests) {
		delete [ ] fRequests;
		fRequests = 0;
	}
}

void WorkerPoolManagerModulePoolManager::Enter(Anything &args)
{
	// make this function block the caller until the worker has finished working
	// to achieve this we create a Mutex and Condition to wait on
	Mutex mx(args["server"].AsString());
	Mutex::ConditionType cond;
	args["mutex"] = (IFAObject *)&mx;
	args["condition"] = (IFAObject *)&cond;
	{
		LockUnlockEntry me(mx);
		WorkerPoolManager::Enter(args);
		// wait on the worker to finish its work and start it with waiting on the condition
		cond.Wait(mx);
	}
}
