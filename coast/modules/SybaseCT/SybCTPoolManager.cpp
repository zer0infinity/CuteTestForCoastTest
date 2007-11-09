/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTPoolManager.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- SybaseWorker -----------------------------------------------
SybaseWorker::SybaseWorker(const char *name)
	: WorkerThread(name)
	, fpCT(NULL)
{
	StartTrace(SybaseWorker.SybaseWorker);
}

SybaseWorker::~SybaseWorker()
{
	StartTrace(SybaseWorker.~SybaseWorker);
	if (fpCT) {
		delete fpCT;
		fpCT = NULL;
	}
}

// one time initialization
void SybaseWorker::DoInit(ROAnything workerInit)
{
	StartTrace(SybaseWorker.DoInit);
	// initialize a new Sybase-CT object for later use;
	// a valid context sould be passed from the PoolManager
	if (!fpCT) {
		CS_CONTEXT *pCtx;
		TraceAny(workerInit, "workerInit");
		pCtx = (CS_CONTEXT *)workerInit["context"].AsIFAObject(0L);
		if (pCtx != NULL) {
			fpCT = new SybCT(pCtx);
		}
	} else {
		SysLog::Error("SybaseWorker::DoInit: trying to initialize SybCT again??");
	}
}

void SybaseWorker::DoProcessWorkload()
{
	StartTrace(SybaseWorker.DoProcessWorkload);
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
		Assert(fpCT != NULL);
		{
			LockUnlockEntry me(*pMx);
			(*pMessages)["RetCode"] = false;
			// do the work
			if ( !fpCT->Open( pMessages, fWork["user"].AsString(), fWork["password"].AsString(), fWork["server"].AsString(), fWork["app"].AsString() ) ) {
				Trace("could not open connection to " << fWork["server"].AsString());
				fpCT->Close(true);
			} else {
				Trace("executing command [" << fWork["query"].AsString() << "]");
				if ( !fpCT->SqlExec( fWork["query"].AsString(), fWork["resultformat"].AsString(), fWork["resultsize"].AsLong() ) ) {
					Trace( "could not execute the sql command");
					fpCT->Close(true);
				} else {
					Anything *pResult = (Anything *)fWork["results"].AsIFAObject(0L);
					Anything *pTitles = (Anything *)fWork["titles"].AsIFAObject(0L);
					Assert(pResult != NULL);
					// get the result
					if (!fpCT->GetResult(*pResult, *pTitles)) {
						Trace("did not get any results");
					} else {
						(*pMessages)["RetCode"] = true;
					}
					fpCT->Close(false);
				}
			}
		}
		// signal that we finished working
		pCond->Signal();
	}
}

// passing of the work
void SybaseWorker::DoWorkingHook(ROAnything workerConfig)
{
	StartTrace(SybaseWorker.DoWorkingHook);
	// for us the work is a valid SQL statement and the user as which it will be executed
	// we have to store the needed information locally because it is a ROAnything
	fWork = workerConfig.DeepClone();
}

void SybaseWorker::DoTerminationRequestHook(ROAnything)
{
	StartTrace(SybaseWorker.DoTerminationRequestHook);
}

void SybaseWorker::DoTerminatedHook()
{
	StartTrace(SybaseWorker.DoTerminatedHook);
}

//---- SybCTPoolManager ------------------------------------------------

SybCTPoolManager::SybCTPoolManager(String name)
	: WorkerPoolManager(name)
	, fRequests(0)
	, fpContext(NULL)
{
	StartTrace(SybCTPoolManager.SybCTPoolManager);
}

SybCTPoolManager::~SybCTPoolManager()
{
	StartTrace(SybCTPoolManager.~SybCTPoolManager);
	Terminate(5L);
	Anything dummy;
	DoDeletePool(dummy);
	if (fpContext) {
		Trace("deleting cs_context");
		SybCT::Finis(fpContext);
		fpContext = NULL;
		// trace messages which occurred without a connection
		while (fContextMessages.GetSize()) {
			SysLog::Warning(String() << fContextMessages[0L].AsCharPtr() << "\n");
			fContextMessages.Remove(0L);
		}
	}
}

bool SybCTPoolManager::Init(const ROAnything config)
{
	StartTrace(SybCTPoolManager.Init);
	bool bRet = false;
	// let the pool initialize a Sybase Context structure
	long concurrentQueries = 5L;
	String strInterfacesPathName;
	if (config.IsDefined("ParallelQueries")) {
		concurrentQueries = config["ParallelQueries"].AsLong(5L);
	}
	if (config.IsDefined("InterfacesPathName")) {
		strInterfacesPathName = config["InterfacesPathName"].AsCharPtr();
	}
	long usePoolStorage = 0L;
	u_long poolStorageSize = 1000L;
	u_long numOfPoolBucketSizes = 20L;
	Trace("Initializing worker pool with " << concurrentQueries << " possible parallel queries");

	// let the pool initialize a Sybase Context structure
	if (SybCT::Init(&fpContext, &fContextMessages, strInterfacesPathName) == CS_SUCCEED) {
		// store context in config to init Workers for this pool correctly
		Anything aCfg;
		aCfg = config.DeepClone();
		aCfg["context"] = (IFAObject *)fpContext;
		TraceAny(aCfg, "pool config");
		Trace("SybCT::Init was successful");
		// initialize the base class
		if (WorkerPoolManager::Init(concurrentQueries, usePoolStorage, poolStorageSize, numOfPoolBucketSizes, aCfg) == 0) {
			Trace("WorkerPoolManager::Init successful");
			bRet = true;
		}
	}
	return bRet;
}

void SybCTPoolManager::DoAllocPool(ROAnything args)
{
	StartTrace(SybCTPoolManager.DoAllocPool);
	// create the pool of worker threads
	fRequests = new SybaseWorker[fPoolSize];
}

WorkerThread *SybCTPoolManager::DoGetWorker(long i)
{
	StartTrace(SybCTPoolManager.DoGetWorker);
	// accessor for one specific worker
	if (fRequests) {
		return &(fRequests[i]);
	}
	return 0;
}

void SybCTPoolManager::DoDeletePool(ROAnything args)
{
	StartTrace(SybCTPoolManager.DoDeletePool);
	// cleanup of the sub-class specific stuff
	// CAUTION: this cleanup method may be called repeatedly..

	if (fRequests) {
		delete [ ] fRequests;
		fRequests = 0;
	}
}

void SybCTPoolManager::Work(Anything &args)
{
	// make this function block the caller until the worker has finished working
	// to achieve this we create a Mutex and Condition to wait on
	Mutex mx(args["server"].AsString());
	Mutex::ConditionType cond;
	args["mutex"] = (IFAObject *)&mx;
	args["condition"] = (IFAObject *)&cond;
	{
		LockUnlockEntry me(mx);
		Enter(args, -1L);
		// wait on the worker to finish its work and start it with waiting on the condition
		cond.Wait(mx);
	}
}
