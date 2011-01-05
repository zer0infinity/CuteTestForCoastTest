/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SegStorAllocatorNewDelete_H
#define _SegStorAllocatorNewDelete_H

#include "ITOStorage.h"
#include "ITOTypeTraits.h"
#include <new>
#include <cassert>
#include "boost/pool/pool.hpp"
#include "boost/pool/detail/singleton.hpp"

namespace Coast {

template<typename T>
class SegStorAllocatorNewDelete {
	typedef boost::pool<ITOStorage::BoostPoolUserAllocatorGlobal> GlobalPoolType;
	template<unsigned N, unsigned NextSize = 32>
	struct CurrentGlobalWrapper {
		CurrentGlobalWrapper() : globalPool(N, NextSize) {
		}

		void * malloc( Allocator *a ) {
			if ( a == Storage::Global() )
				return Storage::fgHooks->LockedAccessFor(globalPool)->malloc();
			else {
				StorageHooks::CurrentPoolTypePtr newTLSPool = Storage::fgHooks->PoolForAlloc(a, N, NextSize);
				return newTLSPool->malloc();
			}
		}

		void free(void *const block, Allocator *a) {
			if ( a == Storage::Global() )
				return Storage::fgHooks->LockedAccessFor(globalPool)->free(block);
			else {
				StorageHooks::CurrentPoolTypePtr newTLSPool = Storage::fgHooks->PoolForFree(a, N, NextSize);
				newTLSPool->free(block);
			}
		}

		GlobalPoolType globalPool;
	};
public:
	static void *operator new( size_t, Allocator *a ) throw () {
		typedef CurrentGlobalWrapper<sizeof(T)> segwrapT;
		typedef boost::details::pool::singleton_default<segwrapT> singleton;
		segwrapT & p = singleton::instance();
		return p.malloc(a);
	}

	static void *operator new( size_t t ) throw () {
		return operator new(t, Storage::Global());
	}

	static void operator delete( void *ptr, Allocator *a ) throw () {
		typedef CurrentGlobalWrapper<sizeof(T)> segwrapT;
		typedef boost::details::pool::singleton_default<segwrapT> singleton;
		segwrapT & p = singleton::instance();
		return p.free(ptr, a);
	}

	static void operator delete( void *ptr ) throw () {
		return operator delete(ptr, static_cast<T*>(ptr)->MyAllocator());
	}
};

}
#endif /* _SegStorAllocatorNewDelete_H */
