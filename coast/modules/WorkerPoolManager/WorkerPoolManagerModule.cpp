/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "WorkerPoolManagerModule.h"
#include "WorkerPoolManagerModulePoolManager.h"
#include "SystemLog.h"
#include "Tracer.h"

//---- WorkerPoolManagerModule ---------------------------------------------------------------
RegisterModule(WorkerPoolManagerModule);

WorkerPoolManagerModule::WorkerPoolManagerModule(const char *name)
	: WDModule(name)
{
	StartTrace(WorkerPoolManagerModule.WorkerPoolManagerModule);
}

WorkerPoolManagerModule::~WorkerPoolManagerModule()
{
	StartTrace(WorkerPoolManagerModule.~WorkerPoolManagerModule);
	Finis();
}

bool WorkerPoolManagerModule::Init(const ROAnything config)
{
	StartTrace(WorkerPoolManagerModule.Init);
	TraceAny(config, "WorkerPoolManagerModule's config");
	long l = 0L;
	ROAnything myCfg;
	if (config.LookupPath(myCfg, "WorkerPoolManagerModule")) {
		// initialize WorkerPools for the listed pools
		if (myCfg.IsDefined("Pools")) {
			ROAnything pools = myCfg["Pools"];
			for (l = 0L; l < pools.GetSize(); l++) {
				ROAnything anySub = pools[l];
				String poolName(pools.SlotName(l));
				TraceAny(anySub, "initializing Pool for " << poolName << " with config");
				WorkerPoolManagerModulePoolManager *pPool = new WorkerPoolManagerModulePoolManager(String("WorkerPoolManagerModulePoolManager:") << poolName);
				if (pPool) {
					Trace("initializing the pool");
					// let the pool initialize...
					if (pPool->Init(anySub)) {
						Trace(String() << poolName << ": init was successful");
						// store reference to pool for later retrieval
						fWorkerPools[poolName] = (IFAObject *)pPool;
					} else {
						delete pPool;
					}
				}
			}
			for (l = 0L; l < fWorkerPools.GetSize(); l++) {
				Trace(String(fWorkerPools.SlotName(l)) << ": starting workers");
				(*(WorkerPoolManager *)fWorkerPools[l].AsIFAObject()).UnblockRequests();
			}
		}
	}
	return true;
}

WorkerPoolManagerModulePoolManager *WorkerPoolManagerModule::GetPoolManager(const char *pPoolName)
{
	StartTrace1(WorkerPoolManagerModule.GetPoolManager, String("searching for ") << pPoolName);
	if (fWorkerPools.IsDefined(pPoolName)) {
		Trace(String(pPoolName) << ": Pool found");
		return (WorkerPoolManagerModulePoolManager *)fWorkerPools[pPoolName].AsIFAObject();
	}
	Trace(String(pPoolName) << ": No Pool found!!");
	SystemLog::Warning(String(pPoolName) << ": No Pool found!!");
	return NULL;
}

bool WorkerPoolManagerModule::Finis()
{
	StartTrace(WorkerPoolManagerModule.Finis);
	while (fWorkerPools.GetSize()) {
		Trace(String(fWorkerPools.SlotName(0L)) << ": stopping workers");
		WorkerPoolManager *pPool = (WorkerPoolManager *)fWorkerPools[0L].AsIFAObject();
		(*pPool).BlockRequests();
		delete pPool;
		fWorkerPools.Remove(0L);
	}

	return true;
}
