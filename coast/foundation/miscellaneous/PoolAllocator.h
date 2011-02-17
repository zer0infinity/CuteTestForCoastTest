/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PoolAllocator_H
#define _PoolAllocator_H

//---- baseclass include -------------------------------------------------
#include "ITOStorage.h"

struct PoolBucket;

class ExcessTrackerElt
{
	friend class PoolAllocatorTest;

	MemTracker *fpTracker;
	ExcessTrackerElt *fpNext;
	size_t fulPayloadSize;

	ExcessTrackerElt &operator=(const ExcessTrackerElt &);

	void SetValues(MemTracker *pTracker, ExcessTrackerElt *pNext, size_t ulPayloadSize);

public:
	ExcessTrackerElt();

	ExcessTrackerElt(MemTracker *pTracker, ExcessTrackerElt *pNext, size_t ulPayloadSize);

	~ExcessTrackerElt();

	void PrintStatistic(long lLevel = -1);

	ul_long GetSizeToPowerOf2(size_t ulWishSize);

	long GetLargestExcessEltBitNum();

	MemTracker *FindTrackerForSize(size_t ulPayloadSize);

	ExcessTrackerElt *InsertTrackerForSize(MemTracker *pTracker, size_t ulPayloadSize);

	void SetId(long lId);

	MemTracker *operator[](size_t ulPayloadSize);

	l_long CurrentlyAllocated();

	void Refresh();

	static void *operator new(size_t size);
	static void *operator new(size_t size, Allocator *);
	static void operator delete(void *d);
};

//---- PoolAllocator ----------------------------------------------------------
//!an allocator that uses a bucketing strategy with pre-allocated memory
//! within Coast to be used as thread-specific allocator
//! is definitely <B>not</B> thread-safe
class PoolAllocator: public Allocator
{
	friend class PoolAllocatorTest;
public:
	/*! create and initialize a pool allocator
		\param poolid use poolid to distinguish more than one pool
		\param poolSize size of pre-allocated pool in kBytes, default 1MByte
		\param maxKindOfBucket number of different allocation units within PoolAllocator, starts at 16 bytes and doubles the size for maxKindOfBucket times. So maxKindOfBucket=10 will give a max usable size of 8192 bytes. */
	PoolAllocator(long poolid, size_t poolSize = 1024, size_t maxKindOfBucket = 10);
	//! destroy a pool only if its empty, i.e. all allocated bytes are freed
	virtual ~PoolAllocator();
	//! implement hook for freeing memory
	virtual void Free(void *vp);
	//! implement hook for freeing memory
	virtual void Free(void *vp, size_t sz);

	/*! Hook to allow allocators to optimize allocation of string buffers for example.
		\param size requested memory size
		\return optimal (maximum) number of bytes which fit into the internal bucket */
	virtual size_t SizeHint(size_t size);

	/*! set an identification for this pool
		\param lId identification for pool
		\return old identifier */
	virtual long SetId(long lId);

	virtual void PrintStatistic(long lLevel = -1);

	l_long  CurrentlyAllocated();
	//! apply this to an empty pool only, rebuilds bucket structure from
	//! ground up, used for request threads after a request is handled
	virtual void Refresh();

	void DumpStillAllocated();

protected:
	void *fPoolMemory;
	size_t fAllocSz;
	size_t fNumOfPoolBucketSizes;
	PoolBucket *fPoolBuckets;

	//!implement hook for allocating memory using bucketing
	virtual void *Alloc(size_t allocSize);

	//auxiliary methods for bucket handling
	MemoryHeader *RemoveHeaderFromBucket(PoolBucket *bucket);
	MemoryHeader *MakeHeaderFromBucket(PoolBucket *bucket, void *lastFree);
	void InsertFreeHeaderIntoBucket(MemoryHeader *mh, PoolBucket *bucket);
	PoolBucket *FindBucketBySize(size_t allocSize);
	void Initialize();

	void IntDumpStillAllocated(MemTracker *pTracker, size_t lSize, size_t lUsableSize);

	// only used for debugging
	MemTracker *fpPoolTotalExcessTracker;
	ExcessTrackerElt *fpExcessTrackerList;
};

#endif
