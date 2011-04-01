/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CACHEHANDLER_H
#define _CACHEHANDLER_H

#include "WDModule.h"

class Mutex;

//---- CacheHandlerModule -----------------------------------------------------------
//! <B>Initializes the global caching structures</B><BR>Configuration: -
//! This Module should be initialized at first to ensure proper functionality of dependent modules
class CacheHandlerModule: public WDModule {
public:
	CacheHandlerModule(const char *name) :
		WDModule(name) {
	}

	virtual bool Init(const ROAnything config);
	virtual bool Finis();
};

//--- CacheLoadPolicy -----------------------------------------------
/****
 Purpose_Begin
 CacheLoadPolicy builds up a cache. It uses an Anything as representation. The contents of the
 cache and the used tags are implementation dependent and must be defined between cache builder
 and cache user (e.g. HTMLTemplateCacheLoader and HTMLTemplateRenderer)
 Purpose_End

 Paramters_Begin
 Group/Key pair identifies the cached object uniquely
 Paramters_End

 Concurrency_Begin
 The build up of the cache is done before the server is accepting requests.
 There are no MT-Issues.
 Concurrency_End

 Collaborations_Begin
 Implementation dependent
 Collaborations_End

 Errorhandling_Begin
 None
 Errorhandling_End

 Assumptions_Begin
 Cache is uniquely identified by Group/Key pair
 Assumptions_End
 ****/
class CacheLoadPolicy {
public:
	virtual ~CacheLoadPolicy() {
	}

	virtual Anything Load(const char *key) {
		return Anything(Coast::Storage::Global());
	}
};

//--- SimpleAnyLoader -----------------------------------------------
class SimpleAnyLoader: public CacheLoadPolicy {
public:
	virtual Anything Load(const char *key);
};

//--- AnythingLoaderPolicy -----------------------------------------------
//! Dummy policy wrap an Anything to cache
class AnythingLoaderPolicy: public CacheLoadPolicy {
public:
	AnythingLoaderPolicy(const Anything &anyToCache) :
		fCachedAny(anyToCache, Coast::Storage::Global()) {
	}
	AnythingLoaderPolicy(const ROAnything roaToCache) :
		fCachedAny(roaToCache.DeepClone(Coast::Storage::Global())) {
	}
	virtual Anything Load(const char *) {
		return fCachedAny;
	}
private:
	Anything fCachedAny;
};

//---- CacheHandler --------------------------------------------------
/****
 Purpose_Begin
 CacheHandler manages a cache for configuration information. This information consists of loaded
 configuration files (Anythings) and preprocessed HTML-template files. The construction of the
 cache is done by a strategy object that is submitted to the load method as parameter.
 Purpose_End

 Paramters_Begin
 CacheLoadPolicy, constructs the cache. Group/Key pair identifies the cached object uniquely
 Paramters_End

 Concurrency_Begin
 The build up of the cache is done before the server is accepting requests. It is distributed through
 ROAnything and installed into clients. There are no MT-Issues during normal operation.
 If the cache has to be reset (not implemented yet), this has to be done while no request is active.
 Concurrency_End

 Collaborations_Begin
 CacheLoadPolicy, Anything
 Collaborations_End

 Errorhandling_Begin
 None
 Errorhandling_End

 Assumptions_Begin
 Cache is uniquely identified by Group/Key pair
 Assumptions_End
 ****/
class CacheHandler: public NotCloned {
	friend class CacheHandlerMutexAllocator;

	// the central cache data structure
	Anything fCache;

	// the one and only cache handler in the system
	static CacheHandler *fgCacheHandler;
	// this mutex protects the cache handler from concurrent access
	static Mutex *fgCacheHandlerMutex;
public:
	CacheHandler() :
		NotCloned("CacheHandler"), fCache(Coast::Storage::Global()) {
	}
	virtual ~CacheHandler() {
		fCache = Anything(Coast::Storage::Global());
	}

	// this operation loads a cache identified by a group, key pair
	// how the cache is built is defined by the CacheLoadPolicy
	// Load checks internally whether this group/key is already loaded
	ROAnything Load(const char *group, const char *key, CacheLoadPolicy *clp);

	ROAnything Reload(const char *group, const char *key, CacheLoadPolicy *clp);

	void Unload(const char *group, const char *key);

	// check for already loaded group/key
	bool IsLoaded(const char *group, const char *key);

	// get a group/key without loading, returns null-roanything if not
	// already loaded
	ROAnything Get(const char *group, const char *key);

	// get a whole group (used for html templates)
	ROAnything GetGroup(const char *group);

	// accessor to the one and only cache handler
	static CacheHandler *Get();
	static void Finis();
};

#endif
