/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "CacheHandler.h"

//--- project modules used -----------------------------------------------------
#include "InitFinisManagerWDBase.h"

//--- standard modules used ----------------------------------------------------
#include "Threads.h"
#include "System.h"
#include "SysLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//--- CacheLoadPolicy -----------------------------------------------
CacheLoadPolicy::CacheLoadPolicy()	{ }
CacheLoadPolicy::~CacheLoadPolicy() { }

Anything CacheLoadPolicy::Load(const char *)
{
	return Anything(Storage::Global());
}

//--- CacheLoadPolicy -----------------------------------------------
AnythingLoaderPolicy::AnythingLoaderPolicy(const Anything &anyToCache)
	: fCachedAny(anyToCache, Storage::Global())
{ }
AnythingLoaderPolicy::~AnythingLoaderPolicy() { }

Anything AnythingLoaderPolicy::Load(const char *)
{
	return fCachedAny;
}

//--- SimpleAnyLoader -----------------------------------------------
SimpleAnyLoader::SimpleAnyLoader() { }
SimpleAnyLoader::~SimpleAnyLoader() { }

Anything SimpleAnyLoader::Load(const char *key)
{
	StartTrace1(SimpleAnyLoader.Load, "trying to load <" << NotNull(key) << ">");
	Anything toLoad(Storage::Global());
	istream *ifp = System::OpenStream(key, "any");

	if (ifp) {
		// found
		toLoad.Import(*ifp, key);
		Trace("loading of <" << NotNull(key) << "> succeeded" );
		SubTraceAny(config, toLoad, "configuration for <" << key << ">");
		delete ifp;
	}
	return toLoad;
}

CacheHandler *CacheHandler::fgCacheHandler = 0;
Mutex *CacheHandler::fgCacheHandlerMutex = 0;

class EXPORTDECL_WDBASE CacheHandlerMutexAllocator : public InitFinisManagerWDBase
{
public:
	CacheHandlerMutexAllocator(unsigned int uiPriority)
		: InitFinisManagerWDBase(uiPriority) {
		IFMTrace("CacheHandlerMutexAllocator created\n");
	}

	~CacheHandlerMutexAllocator()
	{}

	virtual void DoInit() {
		IFMTrace("CacheHandlerMutexAllocator::DoInit\n");
		if ( !CacheHandler::fgCacheHandlerMutex ) {
			CacheHandler::fgCacheHandlerMutex = new Mutex("CacheHandlerMutex", Storage::Global());
		}
	}

	virtual void DoFinis() {
		IFMTrace("CacheHandlerMutexAllocator::DoFinis\n");
		delete CacheHandler::fgCacheHandlerMutex;
		CacheHandler::fgCacheHandlerMutex = 0;
	}
};

static CacheHandlerMutexAllocator *psgCacheHandlerMutexAllocator = new CacheHandlerMutexAllocator(1);

CacheHandler::CacheHandler()
	: NotCloned("CacheHandler")
	, fCache(Storage::Global())
{
}

CacheHandler::~CacheHandler()
{
	fCache = Anything(Storage::Global());
}

void CacheHandler::Finis()
{
	StartTrace(CacheHandler.Finis);
	if  (fgCacheHandler) {
		delete fgCacheHandler;
		fgCacheHandler = 0;
	}
}

ROAnything CacheHandler::Reload(const char *group, const char *key,  CacheLoadPolicy *clp)
{
	MutexEntry me(*fgCacheHandlerMutex);
	me.Use();
	Anything toCache(clp->Load(key), Storage::Global());
	if (! toCache.IsNull() ) {
		fCache[group][key] = toCache;
	}
	return toCache;
}

ROAnything CacheHandler::Load(const char *group, const char *key,  CacheLoadPolicy *clp)
{
	StartTrace1(CacheHandler.Load, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	LockUnlockEntry me(*fgCacheHandlerMutex);
	if ( IsLoaded(group, key) ) {
		return Get(group, key);
	} else {
		Anything toCache(clp->Load(key), Storage::Global());
		if (! toCache.IsNull() ) {
			fCache[group][key] = toCache;
		}
		return toCache;
	}
}

bool CacheHandler::IsLoaded(const char *group, const char *key)
{
	StartTrace1(CacheHandler.IsLoaded, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	if (fCache.IsDefined(group)) {
		return fCache[group].IsDefined(key);
	}
	return false;
}

ROAnything CacheHandler::Get(const char *group, const char *key)
{
	StartTrace1(CacheHandler.Get, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	ROAnything cache(fCache);
	return cache[group][key];
}

ROAnything CacheHandler::GetGroup(const char *group)
{
	StartTrace1(CacheHandler.GetGroup, "group [" << NotNull(group) << "]");
	ROAnything cache(fCache);
	return cache[group];
}

CacheHandler *CacheHandler::Get()
{
	StartTrace(CacheHandler.Get);
	if ( !fgCacheHandler ) {
		LockUnlockEntry me(*fgCacheHandlerMutex);
		// test again if changed while waiting for mutex
		if ( !fgCacheHandler ) {
			fgCacheHandler = new CacheHandler();
		}
	}
	return fgCacheHandler;
}

//---- CacheHandlerModule -----------------------------------------------------------
RegisterModule(CacheHandlerModule);

CacheHandlerModule::CacheHandlerModule(const char *name)
	: WDModule(name)
{
}

CacheHandlerModule::~CacheHandlerModule()
{
}

bool CacheHandlerModule::Init(const ROAnything )
{
	StartTrace(CacheHandlerModule.Init);
	SysLog::WriteToStderr("\tCacheHandler");
	if (CacheHandler::Get()) {
		SysLog::WriteToStderr(". done\n");
		return true;
	}
	SysLog::WriteToStderr(". failed\n");
	return false;
}

bool CacheHandlerModule::Finis()
{
	StartTrace(CacheHandlerModule.Finis);
	CacheHandler::Finis();
	return true;
}
