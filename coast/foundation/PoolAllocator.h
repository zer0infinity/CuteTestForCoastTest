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
	u_long fulPayloadSize;

	ExcessTrackerElt &operator=(const ExcessTrackerElt &);

	void SetValues(MemTracker *pTracker, ExcessTrackerElt *pNext, u_long ulPayloadSize);

public:
	ExcessTrackerElt();

	ExcessTrackerElt(MemTracker *pTracker, ExcessTrackerElt *pNext, u_long ulPayloadSize);

	~ExcessTrackerElt();

	void PrintStatistic(long lLevel = -1);

	ul_long GetSizeToPowerOf2(u_long ulWishSize);

	long GetLargestExcessEltBitNum();

	MemTracker *FindTrackerForSize(u_long ulPayloadSize);

	ExcessTrackerElt *InsertTrackerForSize(MemTracker *pTracker, u_long ulPayloadSize);

	void SetId(long lId);

	MemTracker *operator[](u_long ulPayloadSize);

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
class EXPORTDECL_FOUNDATION PoolAllocator: public Allocator
{
	friend class PoolAllocatorTest;
public:
	/*! create and initialize a pool allocator
		\param poolid use poolid to distinguish more than one pool
		\param poolSize size of pre-allocated pool in kBytes, default 1MByte
		\param maxKindOfBucket number of different allocation units within PoolAllocator, starts at 16 bytes and doubles the size for maxKindOfBucket times. So maxKindOfBucket=10 will give a max usable size of 8192 bytes. */
	PoolAllocator(long poolid, u_long poolSize = 1024, u_long maxKindOfBucket = 10);
	//! destroy a pool only if its empty, i.e. all allocated bytes are freed
	virtual ~PoolAllocator();
	//! implement hook for freeing memory
	virtual size_t Free(void *vp);
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
	u_long fAllocSz;
	u_long fNumOfPoolBucketSizes;
	PoolBucket *fPoolBuckets;

	//!implement hook for allocating memory using bucketing
	virtual void *Alloc(u_long allocSize);

	//auxiliary methods for bucket handling
	MemoryHeader *RemoveHeaderFromBucket(PoolBucket *bucket);
	MemoryHeader *MakeHeaderFromBucket(PoolBucket *bucket, void *lastFree);
	void InsertFreeHeaderIntoBucket(MemoryHeader *mh, PoolBucket *bucket);
	PoolBucket *FindBucketBySize(u_long allocSize);
	void Initialize();

	void IntDumpStillAllocated(MemTracker *pTracker, u_long lSize, u_long lUsableSize);

	// only used for debugging
	MemTracker *fpPoolTotalTracker, *fpPoolTotalExcessTracker;
	ExcessTrackerElt *fpExcessTrackerList;
};

#endif
