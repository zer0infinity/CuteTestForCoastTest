/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "CacheHandler.h"
#include "InitFinisManagerWDBase.h"
#include "Threads.h"
#include "SystemFile.h"
#include "SystemLog.h"
#include "AnyUtils.h"

//--- CacheLoadPolicy -----------------------------------------------
Anything CacheLoadPolicy::Load(const char *) {
	return Anything(Coast::Storage::Global());
}

//--- CacheLoadPolicy -----------------------------------------------
AnythingLoaderPolicy::AnythingLoaderPolicy(const Anything &anyToCache) :
	fCachedAny(anyToCache, Coast::Storage::Global()) {
}

AnythingLoaderPolicy::AnythingLoaderPolicy(const ROAnything roaToCache) :
	fCachedAny(roaToCache.DeepClone(Coast::Storage::Global())) {
}

Anything AnythingLoaderPolicy::Load(const char *) {
	return fCachedAny;
}

//--- SimpleAnyLoader -----------------------------------------------
Anything SimpleAnyLoader::Load(const char *key) {
	StartTrace1(SimpleAnyLoader.Load, "trying to load <" << NotNull(key) << ">");
	Anything toLoad(Coast::Storage::Global());
	std::istream *ifp = Coast::System::OpenStream(key, "any");

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

class CacheHandlerMutexAllocator: public InitFinisManagerWDBase {
public:
	CacheHandlerMutexAllocator(unsigned int uiPriority) :
		InitFinisManagerWDBase(uiPriority) {
		IFMTrace("CacheHandlerMutexAllocator created\n");
	}

	virtual void DoInit() {
		IFMTrace("CacheHandlerMutexAllocator::DoInit\n");
		if (!CacheHandler::fgCacheHandlerMutex) {
			CacheHandler::fgCacheHandlerMutex = new Mutex("CacheHandlerMutex", Coast::Storage::Global());
		}
	}

	virtual void DoFinis() {
		IFMTrace("CacheHandlerMutexAllocator::DoFinis\n");
		delete CacheHandler::fgCacheHandlerMutex;
		CacheHandler::fgCacheHandlerMutex = 0;
	}
};

static CacheHandlerMutexAllocator *psgCacheHandlerMutexAllocator = new CacheHandlerMutexAllocator(1);

CacheHandler::CacheHandler() :
	NotCloned("CacheHandler"), fCache(Coast::Storage::Global()) {
}

CacheHandler::~CacheHandler() {
	fCache = Anything(Coast::Storage::Global());
}

void CacheHandler::Finis() {
	StartTrace(CacheHandler.Finis);
	if (fgCacheHandler) {
		delete fgCacheHandler;
		fgCacheHandler = 0;
	}
}

ROAnything CacheHandler::Reload(const char *group, const char *key, CacheLoadPolicy *clp) {
	LockUnlockEntry me(*fgCacheHandlerMutex);
	Anything toCache(clp->Load(key), fCache.GetAllocator());
	if (!toCache.IsNull()) {
		if (IsLoaded(group, key)) {
			//! \note  never replace a cached anything because tracking ROAnything's will not detect the changed Impl!
			AnyUtils::AnyMerge(fCache[group][key], toCache, true);
		} else {
			fCache[group][key] = toCache;
		}
	}
	return Get(group, key);
}

ROAnything CacheHandler::Load(const char *group, const char *key, CacheLoadPolicy *clp) {
	StartTrace1(CacheHandler.Load, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	LockUnlockEntry me(*fgCacheHandlerMutex);
	if (IsLoaded(group, key)) {
		return Get(group, key);
	} else {
		Anything toCache(clp->Load(key), Coast::Storage::Global());
		if (!toCache.IsNull()) {
			fCache[group][key] = toCache;
		}
		return toCache;
	}
}

bool CacheHandler::IsLoaded(const char *group, const char *key) {
	StartTrace1(CacheHandler.IsLoaded, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	if (fCache.IsDefined(group)) {
		return fCache[group].IsDefined(key);
	}
	return false;
}

ROAnything CacheHandler::Get(const char *group, const char *key) {
	StartTrace1(CacheHandler.Get, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	ROAnything cache(fCache);
	return cache[group][key];
}

ROAnything CacheHandler::GetGroup(const char *group) {
	StartTrace1(CacheHandler.GetGroup, "group [" << NotNull(group) << "]");
	ROAnything cache(fCache);
	return cache[group];
}

CacheHandler *CacheHandler::Get() {
	StartTrace(CacheHandler.Get);
	if (!fgCacheHandler && fgCacheHandlerMutex) {
		LockUnlockEntry me(*fgCacheHandlerMutex);
		// test again if changed while waiting for mutex
		if (!fgCacheHandler) {
			fgCacheHandler = new (Coast::Storage::Global()) CacheHandler();
		}
	}
	return fgCacheHandler;
}
//---- CacheHandlerModule -----------------------------------------------------------
RegisterModule(CacheHandlerModule);

bool CacheHandlerModule::Init(const ROAnything) {
	StartTrace(CacheHandlerModule.Init);
	SystemLog::WriteToStderr("\tCacheHandler");
	if (CacheHandler::Get()) {
		SystemLog::WriteToStderr(". done\n");
		return true;
	}
	SystemLog::WriteToStderr(". failed\n");
	return false;
}

bool CacheHandlerModule::Finis() {
	StartTrace(CacheHandlerModule.Finis);
	CacheHandler::Finis();
	return true;
}
