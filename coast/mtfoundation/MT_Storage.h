/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MT_Storage_H
#define _MT_Storage_H

#include "config_mtfoundation.h"
#include "SystemAPI.h"
#include "MemHeader.h"
#include "ITOStorage.h"

class Mutex;

#ifdef MEM_DEBUG

//! multithreading safe memory allocation tracker (only available if MEM_DEBUG enabled)
class EXPORTDECL_MTFOUNDATION MT_MemTracker : public MemTracker
{
public:
	//!creates system dependent mutex to provide locking without allocation of strings
	MT_MemTracker(const char *name);
	//!destroys system dependent mutex
	virtual ~MT_MemTracker();

	//!tracks allocation; locks mutex
	virtual void TrackAlloc(u_long allocSz);
	//!tracks frees: locks mutex
	virtual void TrackFree(u_long allocSz);
	//!print allocation/deallocation statistics
	virtual void PrintStatistic();
	//!returns currently allocated bytes
	l_long  CurrentlyAllocated();
protected:
	//!system dependent mutex handle to avoid allocation of string memory
	MUTEX fMutex;
};
#endif

//!Multi Threading aware storage management using thread local store pools
class EXPORTDECL_MTFOUNDATION MT_Storage
{
public:
	/*! allocates a pool allocator which is used as thread local store
		\param poolStorageSize size of the pool in 1k Bytes default is 1MB
		\param numOfPoolBucketSizes number of different sizes of memory chunks the allocator aligns allocation to; the size is a power of 2
		\param lPoolId optionally give an identifier for the pool
		\return a pool allocator with poolStorageSize * 1KB size memory and allocation divided in 2^numOfPoolBucketSizes chunks if ok; else NULL */
	static Allocator *MakePoolAllocator(u_long poolStorageSize = 1000, u_long numOfPoolBucketSizes = 20, long lPoolId = 0);

	//! register allocator in the current threads local store (true means success)
	static bool RegisterThread(Allocator *wdallocator);

	//! must be called before threading is activated
	static void Initialize(); // register MT_Storage capability with Storage

	//! must be called before threading is deactivated
	static void Finalize(); // de-register MT_Storage capability

	//!does ref counting for allocators with mutex protection; global mutex might be slightly inefficient
	static void RefAllocator(Allocator *wdallocator);

	//!does ref counting for allocators with mutex protection; global mutex might be slightly inefficient
	static void UnrefAllocator(Allocator *wdallocator);

	static THREADKEY fgAllocatorKey;
	static bool fgInitialized;
	static Mutex *fgAllocatorInit;
};

#endif
