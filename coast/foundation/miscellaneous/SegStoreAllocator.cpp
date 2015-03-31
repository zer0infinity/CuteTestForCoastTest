/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SegStoreAllocator.h"
#include <iostream>

SegStoreAllocator::~SegStoreAllocator() {
	allocPoolMap.clear();
}

void SegStoreAllocator::PrintStatistic(long lLevel) {
	long lStatisticLevel = ( ( lLevel >= 0 ) ? lLevel : coast::storage::GetStatisticLevel() );
	if ( lStatisticLevel >= 1 ) {
	}
}

void* SegStoreAllocator::Alloc(size_t sz) {
	AllocPoolMapping::iterator it;
	CurrentPoolTypePtr aPool;

	if ((it = allocPoolMap.find(sz)) != allocPoolMap.end()) {
		aPool = it->second;
	} else {
		aPool = CurrentPoolTypePtr(new CurrentPoolType(sz, 32));
		allocPoolMap.insert(std::make_pair(sz, aPool));
	}

	return aPool->malloc();
}

void SegStoreAllocator::Free(void *p, size_t sz) {
	AllocPoolMapping::iterator it;
	CurrentPoolTypePtr aPool;

	if ((it = allocPoolMap.find(sz)) != allocPoolMap.end()) {
		aPool = it->second;
	}

	aPool->free(p);
}
