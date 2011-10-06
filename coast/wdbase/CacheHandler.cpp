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

CacheHandlerPrototype::CacheHandlerPrototype() :
		NotCloned("CacheHandler"), fCache(Coast::Storage::Global()), fCacheHandlerMutex("CacheHandlerMutex", Coast::Storage::Global()) {
}

ROAnything CacheHandlerPrototype::Reload(const char *group, const char *key, CacheLoadPolicy *clp) {
	StartTrace1(CacheHandlerPrototype.Reload, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	LockUnlockEntry me(fCacheHandlerMutex);
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

ROAnything CacheHandlerPrototype::Load(const char *group, const char *key, CacheLoadPolicy *clp) {
	StartTrace1(CacheHandlerPrototype.Load, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	LockUnlockEntry me(fCacheHandlerMutex);
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

bool CacheHandlerPrototype::IsLoaded(const char *group, const char *key) {
	StartTrace1(CacheHandlerPrototype.IsLoaded, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	if (fCache.IsDefined(group)) {
		return fCache[group].IsDefined(key);
	}
	return false;
}

void CacheHandlerPrototype::Unload(const char *group, const char *key) {
	StartTrace1(CacheHandlerPrototype.Unload, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	LockUnlockEntry me(fCacheHandlerMutex);
	if ( IsLoaded(group, key) ) {
		SlotCleaner::Operate(fCache, String(group).Append('.').Append(key));
	}
}

ROAnything CacheHandlerPrototype::Get(const char *group, const char *key) {
	StartTrace1(CacheHandlerPrototype.Get, "group [" << NotNull(group) << "] key [" << NotNull(key) << "]");
	ROAnything cache(fCache);
	return cache[group][key];
}

ROAnything CacheHandlerPrototype::GetGroup(const char *group) {
	StartTrace1(CacheHandlerPrototype.GetGroup, "group [" << NotNull(group) << "]");
	ROAnything cache(fCache);
	return cache[group];
}

bool CacheHandlerPrototype::Init(const ROAnything) {
	LockUnlockEntry me(fCacheHandlerMutex);
	return true;
}

bool CacheHandlerPrototype::Finis() {
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

