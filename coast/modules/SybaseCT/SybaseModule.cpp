/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybaseModule.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"
#include "SybCTPoolManager.h"
#include "SybCTDAImpl.h"

//---- SybaseModule ---------------------------------------------------------------
RegisterModule(SybaseModule);

SybaseModule::SybaseModule(const char *name)
	: WDModule(name)
{
	StartTrace(SybaseModule.SybaseModule);
}

SybaseModule::~SybaseModule()
{
	StartTrace(SybaseModule.~SybaseModule);
	Finis();
}

bool SybaseModule::Init(const Anything &config)
{
	StartTrace(SybaseModule.Init);

	long l = 0L;
	Anything myCfg;
	if (config.LookupPath(myCfg, "SybaseModule")) {
		// initialize WorkerPools for the listed servers
		if (myCfg.IsDefined("Servers")) {
			Anything &servers = myCfg["Servers"];
			for (l = 0L; l < servers.GetSize(); l++) {
				Anything &anySub = servers[l];
				String srvName(servers.SlotName(l));
				TraceAny(anySub, "initializing Pool for " << srvName << " with config");
				SybCTPoolManager *pPool = new SybCTPoolManager(String("SybaseCTPool:") << srvName);
				if (pPool) {
					Trace("initializing the pool");
					// let the pool initialize a Sybase Context structure
					if (pPool->Init(anySub)) {
						Trace(String() << srvName << ": init was successful");
						// store reference to pool for later retrieval
						fWorkerPools[srvName] = (IFAObject *)pPool;
					} else {
						delete pPool;
					}
				}
			}
			for (l = 0L; l < fWorkerPools.GetSize(); l++) {
				Trace(String(fWorkerPools.SlotName(l)) << ": starting workers");
				(*(SybCTPoolManager *)fWorkerPools[l].AsIFAObject()).UnblockRequests();
			}
		}
		// initialize standard SybCTDAImpl
		SybCTDAImpl::Init(config);
	}
	return true;
}

SybCTPoolManager *SybaseModule::GetPoolManager(const char *pServerName)
{
	StartTrace1(SybaseModule.GetPoolManager, String("searching for ") << pServerName);
	if (fWorkerPools.IsDefined(pServerName)) {
		Trace(String(pServerName) << ": Pool found");
		return (SybCTPoolManager *)fWorkerPools[pServerName].AsIFAObject();
	}
	Trace(String(pServerName) << ": No Pool found!!");
	SysLog::Warning(String(pServerName) << ": No Pool found!!");
	return NULL;
}

bool SybaseModule::Finis()
{
	StartTrace(SybaseModule.Finis);
	while (fWorkerPools.GetSize()) {
		Trace(String(fWorkerPools.SlotName(0L)) << ": stopping workers");
		SybCTPoolManager *pPool = (SybCTPoolManager *)fWorkerPools[0L].AsIFAObject();
		(*pPool).BlockRequests();
		delete pPool;
		fWorkerPools.Remove(0L);
	}
	// de-initialize standard SybCTDAImpl
	SybCTDAImpl::Finis();

	return true;
}
