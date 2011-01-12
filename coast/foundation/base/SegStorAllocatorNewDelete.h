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
#include "AllocatorPool.h"

namespace Coast {

template<typename T>
class SegStorAllocatorNewDelete {
	template<unsigned N, unsigned NextSize = 32>
	struct CurrentGlobalWrapper {
		CurrentGlobalWrapper() : globalPool(N, NextSize) {
		}

		typedef boost::pool<ITOStorage::BoostPoolUserAllocatorGlobal> GlobalPoolType;
		typedef Coast::Memory::MyPool CurrentPoolType;
		typedef boost::shared_ptr<CurrentPoolType> CurrentPoolTypePtr;
		typedef std::map<Allocator*, CurrentPoolTypePtr> AllocPoolMapping;
		AllocPoolMapping allocPoolMap;

		CurrentPoolTypePtr PoolForAlloc(Allocator* a, std::size_t nrequested_size, std::size_t nnext_size) {
			AllocPoolMapping::iterator it;
			CurrentPoolTypePtr aPool;
			if ((it = allocPoolMap.find(a)) != allocPoolMap.end()) {
				aPool = it->second;
			} else {
				aPool = CurrentPoolTypePtr(new CurrentPoolType(a, nrequested_size, nnext_size));
				allocPoolMap.insert(std::make_pair(a, aPool));
			}
			return aPool;
		}

		CurrentPoolTypePtr PoolForFree(Allocator* a, std::size_t nrequested_size, std::size_t nnext_size) {
			AllocPoolMapping::iterator it;
			CurrentPoolTypePtr aPool;
			if ((it = allocPoolMap.find(a)) != allocPoolMap.end()) {
				aPool = it->second;
			} else {
				//TODO
			}
			return aPool;
		}

		void * malloc( Allocator *a ) {
			if ( a == Storage::Global() )
				return Storage::fgHooks->LockedAccessFor(globalPool)->malloc();
			else {
				CurrentPoolTypePtr newTLSPool = PoolForAlloc(a, N, NextSize);
				return newTLSPool->pmalloc();
			}
		}

		void free(void *const block, Allocator *a) {
			if ( a == Storage::Global() )
				return Storage::fgHooks->LockedAccessFor(globalPool)->free(block);
			else {
				CurrentPoolTypePtr newTLSPool = PoolForFree(a, N, NextSize);
				newTLSPool->pfree(block);
			}
		}
		GlobalPoolType globalPool;
	};

public:
	static void *operator new( size_t, Allocator *a ) {
		typedef CurrentGlobalWrapper<sizeof(T)> segwrapT;
		typedef boost::details::pool::singleton_default<segwrapT> singleton;
		segwrapT & p = singleton::instance();
		return p.malloc(a);
	}

	static void *operator new( size_t t ) {
		return operator new(t, Storage::Global());
	}

	static void operator delete( void *ptr, Allocator *a ) {
		typedef CurrentGlobalWrapper<sizeof(T)> segwrapT;
		typedef boost::details::pool::singleton_default<segwrapT> singleton;
		segwrapT & p = singleton::instance();
		return p.free(ptr, a);
	}

	static void operator delete( void *ptr ) {
		return operator delete(ptr, static_cast<T*>(ptr)->MyAllocator());
	}
};

}
#endif /* _SegStorAllocatorNewDelete_H */
