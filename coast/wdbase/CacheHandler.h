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
#include "Threads.h"

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

//! CacheLoadPolicy builds up a cache.
/*! It uses an Anything as representation. The contents of the
 cache and the used tags are implementation dependent and must be defined between cache builder
 and cache user (e.g. HTMLTemplateCacheLoader and HTMLTemplateRenderer)

 Group/Key pair identifies the cached object uniquely

 The build up of the cache is done before the server is accepting requests.
 There are no MT-Issues.

 Cache is uniquely identified by Group/Key pair
 */
class CacheLoadPolicy {
public:
	virtual ~CacheLoadPolicy() {
	}

	virtual Anything Load(const char *key) {
		return Anything(Coast::Storage::Global());
	}
};

class SimpleAnyLoader: public CacheLoadPolicy {
public:
	virtual Anything Load(const char *key);
};

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

#include <boost/pool/detail/singleton.hpp>
#include <boost/shared_ptr.hpp>

//! CacheHandlerPrototype manages a cache for configuration information
/*!
 This information consists of loaded
 configuration files (Anythings) and preprocessed HTML-template files. The construction of the
 cache is done by a strategy object that is submitted to the load method as parameter.

 CacheLoadPolicy, constructs the cache. Group/Key pair identifies the cached object uniquely

 The build up of the cache is done before the server is accepting requests. It is distributed through
 ROAnything and installed into clients. There are no MT-Issues during normal operation.
 If the cache has to be reset (not implemented yet), this has to be done while no request is active.

 Cache is uniquely identified by Group/Key pair
*/
class CacheHandlerPrototype: public NotCloned {
	typedef Mutex MutexType;
	// the central cache data structure
	Anything fCache;

	// this mutex protects the cache handler from concurrent access
	MutexType fCacheHandlerMutex;
public:
	CacheHandlerPrototype();
	virtual ~CacheHandlerPrototype() {
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

	bool Init(const ROAnything);
	bool Finis();
};
typedef boost::details::pool::singleton_default<CacheHandlerPrototype> CacheHandler;

#endif
