/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "CacheHandler.h"
#include "SystemFile.h"
#include "SystemLog.h"
#include "AnyUtils.h"
#include "InitFinisManager.h"

Anything SimpleAnyLoader::Load(const char *key) {
	StartTrace1(SimpleAnyLoader.Load, "trying to load <" << NotNull(key) << ">");
	Anything toLoad(coast::storage::Global());
	std::istream *ifp = coast::system::OpenStream(key, "any");

	if (ifp) {
		// found
		toLoad.Import(*ifp, key);
		Trace("loading of <" << NotNull(key) << "> succeeded" );
		SubTraceAny(config, toLoad, "configuration for <" << key << ">");
		delete ifp;
	}
	return toLoad;
}

CacheHandlerImpl::CacheHandlerImpl() :
		NotCloned("CacheHandler"), fCache(Anything::ArrayMarker(), coast::storage::Global()), fCacheHandlerMutex("CacheHandlerMutex", coast::storage::Global()) {
	InitFinisManager::IFMTrace("CacheHandler::Initialized\n");
}

CacheHandlerImpl::~CacheHandlerImpl() {
	fCache.clear();
	InitFinisManager::IFMTrace("CacheHandler::Finalized\n");
}

ROAnything CacheHandlerImpl::Reload(const char *group, const char *key, CacheLoadPolicy *clp) {
	StartTrace1(CacheHandlerImpl.Reload, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	Anything toCache(clp->Load(key), fCache.GetAllocator());
	LockUnlockEntry me(fCacheHandlerMutex);
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

ROAnything CacheHandlerImpl::Load(const char *group, const char *key, CacheLoadPolicy *clp) {
	StartTrace1(CacheHandlerImpl.Load, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	if (not IsLoaded(group, key)) {
		Anything toCache(clp->Load(key), fCache.GetAllocator());
		if (!toCache.IsNull()) {
			LockUnlockEntry me(fCacheHandlerMutex);
			fCache[group][key] = toCache;
		}
	}
	return Get(group, key);
}

bool CacheHandlerImpl::IsLoaded(const char *group, const char *key) {
	StartTrace1(CacheHandlerImpl.IsLoaded, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	return ROAnything(fCache).IsDefined(group) && ROAnything(fCache)[group].IsDefined(key);
}

void CacheHandlerImpl::Unload(const char *group, const char *key) {
	StartTrace1(CacheHandlerImpl.Unload, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	if ( IsLoaded(group, key) ) {
		LockUnlockEntry me(fCacheHandlerMutex);
		SlotCleaner::Operate(fCache, String(group).Append('.').Append(key));
	}
}

ROAnything CacheHandlerImpl::Get(const char *group, const char *key) {
	StartTrace1(CacheHandlerImpl.Get, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	ROAnything cache(fCache);
	return cache[group][key];
}

ROAnything CacheHandlerImpl::GetGroup(const char *group) {
	StartTrace1(CacheHandlerImpl.GetGroup, "group [" << NotNull(group) << "]");
	ROAnything cache(fCache);
	return cache[group];
}

bool CacheHandlerImpl::Init(const ROAnything) {
	LockUnlockEntry me(fCacheHandlerMutex);
	return true;
}

bool CacheHandlerImpl::Finis() {
	LockUnlockEntry me(fCacheHandlerMutex);
	fCache.clear();
	return true;
}

RegisterModule(CacheHandlerModule);

bool CacheHandlerModule::Init(const ROAnything config) {
	StartTrace(CacheHandlerModule.Init);
	SystemLog::WriteToStderr("\tCacheHandler");
	if (CacheHandler::instance().Init(config)) {
		SystemLog::WriteToStderr(". done\n");
		return true;
	}
	SystemLog::WriteToStderr(". failed\n");
	return false;
}

bool CacheHandlerModule::Finis() {
	StartTrace(CacheHandlerModule.Finis);
	return CacheHandler::instance().Finis();
}
