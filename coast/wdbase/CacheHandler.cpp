/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "CacheHandler.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
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
#if !defined (_AIX)   //static Initialisation problem
	StartTrace1(SimpleAnyLoader.Load, "trying to load <" << NotNull(key) << ">");
#endif
	Anything toLoad(Storage::Global());
	istream *ifp = System::OpenStream(key, "any");

	if (ifp) {
		// found
		toLoad.Import(*ifp, key);
#if !defined (_AIX)   //static Initialisation problem
		Trace("loading of <" << NotNull(key) << "> suceeded" );
		SubTraceAny(config, toLoad, "configuration for <" << key << ">");
#endif
		delete ifp;
	}
	return toLoad;
}

CacheHandler *CacheHandler::fgCacheHandler = 0;
Mutex *CacheHandler::fgCacheHandlerMutex = 0;

CacheHandler::CacheHandler() : NotCloned("CacheHandler"), fCache(Storage::Global())
{
}

CacheHandler::~CacheHandler()
{
	fCache = Anything(Storage::Global());
}

void CacheHandler::Finis()
{
	if  (fgCacheHandler) {
		delete fgCacheHandler;
		fgCacheHandler = 0;
		delete fgCacheHandlerMutex;
		fgCacheHandlerMutex = 0;
	}
}

ROAnything CacheHandler::Load(const char *group, const char *key,  CacheLoadPolicy *clp)
{
	MutexEntry me(*fgCacheHandlerMutex);
	me.Use();
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
	if (fCache.IsDefined(group)) {
		return fCache[group].IsDefined(key);
	}
	return false;
}

ROAnything CacheHandler::Get(const char *group, const char *key)
{
	ROAnything cache(fCache);
	return cache[group][key];
}

ROAnything CacheHandler::GetGroup(const char *group)
{
	ROAnything cache(fCache);
	return cache[group];
}

CacheHandler *CacheHandler::Get()
{
	if ( !fgCacheHandler) {
		if ( ! fgCacheHandlerMutex ) {
			fgCacheHandlerMutex = new Mutex("CacheHandler");
		}
		MutexEntry me(*fgCacheHandlerMutex);
		me.Use();

		if ( ! fgCacheHandler ) {	// test again if changed while waiting for mutex
			fgCacheHandler = new CacheHandler();
		}
	}
	return fgCacheHandler;
}

//---- CacheHandlerModule -----------------------------------------------------------
RegisterModule(CacheHandlerModule);

CacheHandlerModule::CacheHandlerModule(const char *name) : WDModule(name)
{
}

CacheHandlerModule::~CacheHandlerModule()
{
}

bool CacheHandlerModule::Init(const ROAnything )
{
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
	SysLog::WriteToStderr("\tTerminating CacheHandler");
	CacheHandler::Finis();
	SysLog::WriteToStderr(" done\n");
	return true;
}
