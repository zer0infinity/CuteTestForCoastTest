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
#include "SybCTnewDAImpl.h"

//---- SybaseModule ---------------------------------------------------------------
RegisterModule(SybaseModule);

SybaseModule::SybaseModule(const char *name)
	: WDModule(name)
	, fHasDAImpls(false)
	, fHasNewDAImpls(false)
{
	StartTrace(SybaseModule.SybaseModule);
}

SybaseModule::~SybaseModule()
{
	StartTrace(SybaseModule.~SybaseModule);
	Finis();
}

bool SybaseModule::Init(const ROAnything config)
{
	StartTrace(SybaseModule.Init);

	long l = 0L;
	ROAnything myCfg;
	if (config.LookupPath(myCfg, "SybaseModule")) {
		TraceAny(myCfg, "SybaseModuleConfig");
		// initialize WorkerPools for the listed servers
		if (myCfg.IsDefined("SybCTPoolDAImpl")) {
			ROAnything servers = myCfg["SybCTPoolDAImpl"];
			for (l = 0L; l < servers.GetSize(); l++) {
				ROAnything anySub = servers[l];
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
		if ( myCfg.IsDefined("SybCTDAImpl") ) {
			// initialize standard SybCTDAImpl
			fHasDAImpls = SybCTDAImpl::Init(config);
		}
		if ( myCfg.IsDefined("SybCTnewDAImpl") ) {
			// initialize SybCTnewDAImpls
			Trace("initializing SybCTnewDAImpl");
			fHasNewDAImpls = SybCTnewDAImpl::Init(config);
		}
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
	if ( fHasDAImpls && SybCTDAImpl::fgInitialized ) {
		// de-initialize SybCTDAImpl
		SybCTDAImpl::Finis();
	}
	if ( fHasNewDAImpls && SybCTnewDAImpl::fgInitialized ) {
		// de-initialize SybCTnewDAImpls
		SybCTnewDAImpl::Finis();
	}
	return true;
}
