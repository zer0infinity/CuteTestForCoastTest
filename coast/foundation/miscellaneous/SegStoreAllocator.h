/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SegStorAllocator_H
#define _SegStorAllocator_H

#include "ITOStorage.h"
#include <boost/shared_ptr.hpp>
#include <boost/pool/pool.hpp>
#include <map>

class SegStoreAllocator : public Allocator {
	typedef boost::pool<itostorage::BoostPoolUserAllocatorGlobal> CurrentPoolType;
	typedef boost::shared_ptr<CurrentPoolType> CurrentPoolTypePtr;
	typedef std::map<size_t, CurrentPoolTypePtr> AllocPoolMapping;

	AllocPoolMapping allocPoolMap;

	//FIXME actually breaks LSP
	void Free(void *vp) {}
public:
	SegStoreAllocator(long allocatorid) : Allocator(allocatorid) {}
	~SegStoreAllocator();

	//!print out the allocators statistics
	void PrintStatistic(long lLevel = -1);

	//!returns the currently allocated bytes
	ul_long CurrentlyAllocated() {
		return 0;
	}

	virtual void *Malloc(size_t size) {
		return Alloc(size);
	}

	void *Alloc(size_t sz);
	void Free(void *p, size_t sz);
};
#endif /* _SegStorAllocator_H */
