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

//---- PoolAllocator ----------------------------------------------------------
//!an allocator that uses a bucketing strategy with pre-allocated memory
//! within Coast to be used as thread-specific allocator
//! is definitely <B>not</B> thread-safe
class EXPORTDECL_FOUNDATION PoolAllocator: public Allocator
{
public:
	//! create and initialize a pool allocator
	//! \param poolid use poolid to distinguish more than one pool
	//! \param poolSize size of pre-allocated pool in kBytes, default 1MByte
	PoolAllocator(long poolid, u_long poolSize = 1024, u_long maxKindOfBucket = 10); // 1k * poolSize
	//! destroy a pool only if its empty, i.e. all allocated bytes are freed
	virtual ~PoolAllocator();
	//! implement hook for freeing memory
	virtual void  Free(void *vp);
	//! hook to allow allocators to optimize allocation of string buffers
	virtual size_t SizeHint(size_t size);

	/*! set an identification for this pool
		\param lId identification for pool
		\return old identifier */
	virtual long SetId(long lId);

	virtual void PrintStatistic();
#ifdef MEM_DEBUG
	l_long  CurrentlyAllocated();
#endif
	//! apply this to an empty pool only, rebuilds bucket structure from
	//! ground up, used for request threads after a request is handled
	virtual void Refresh();

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

	// only used for debugging
	MemTrackDef(fPoolTracker) // semicolons are set by macro already ;
	MemTrackDef(fExcessTracker) // otherwise some compilers complain about empty decls
};

#endif
