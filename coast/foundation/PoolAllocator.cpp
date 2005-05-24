/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "PoolAllocator.h"

//--- standard modules used ----------------------------------------------------
#include "MemHeader.h"
#include "SysLog.h"
#include "StringStream.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

#if !defined(__SUNPRO_CC) ||  __SUNPRO_CC < 0x500
extern void *operator new(size_t size, void *vp);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
extern void operator delete(void *ptr, void *vp);
#endif
extern void operator delete(void *ptr);
#endif

//---- PoolAllocator ------------------------------------------
struct PoolBucket {
	u_long fSize;
	u_long fUsableSize;
	void *fFirstFree;
};

PoolAllocator::PoolAllocator(long poolid, u_long poolSize, u_long maxPoolBuckets)
	: Allocator(poolid)
	, fNumOfPoolBucketSizes(maxPoolBuckets)
	MemTrackInit(fPoolTracker)
	MemTrackInit(fExcessTracker)
{
	fAllocSz = poolSize * 1024;
	fPoolMemory = ::calloc(fAllocSz, 1);
	fPoolBuckets = (PoolBucket *)::calloc(fNumOfPoolBucketSizes + 1, sizeof(PoolBucket));

	if (!fPoolMemory || !fPoolBuckets) {
		static const char crashmsg[] = "FATAL: PoolAllocator::PoolAllocator calloc failed. I will crash :-(\n";
		SysLog::WriteToStderr(crashmsg, sizeof(crashmsg));

		// We might never see the following because we have no memory.
		String msg("PoolAllocator: ");
		msg << "allocation of PoolStorage: " << (long)poolSize << ", " << (long)maxPoolBuckets << " failed";
		SysLog::Error(msg);
		Unref(); // signal allocation failure
		return;
	}
	Initialize();

#ifdef MEM_DEBUG
	fPoolTracker.SetId(poolid);
	fExcessTracker.SetId(poolid);
#endif
}

long PoolAllocator::SetId(long lId)
{
#ifdef MEM_DEBUG
	fPoolTracker.SetId(lId);
	fExcessTracker.SetId(lId);
#endif
	return Allocator::SetId(lId);
}

void PoolAllocator::Initialize()
{
	// initialize data structures used by the allocator
	long sz = 16;

	for (long i = 0; i < (long)fNumOfPoolBucketSizes; i++) {
		fPoolBuckets[i].fSize = sz;
		fPoolBuckets[i].fUsableSize = sz - MemoryHeader::AlignedSize();	// optimization
		fPoolBuckets[i].fFirstFree = NULL;

		sz *= 2;
	}
	fPoolBuckets[fNumOfPoolBucketSizes].fSize = 0;
	fPoolBuckets[fNumOfPoolBucketSizes].fFirstFree = ((char *)fPoolMemory) + fAllocSz;
}

PoolAllocator::~PoolAllocator()
{
	MemTrackStatIfAllocated(fPoolTracker);
	MemTrackStatIfAllocated(fExcessTracker);
	MemTrackStillAllocatedException(fPoolTracker, "PoolAllocator::~PoolAllocator");
	::free(fPoolBuckets);
	::free(fPoolMemory);
}

void PoolAllocator::Free(void *vp)
{
	MemoryHeader *header = RealMemStart(vp);

	if ( header ) {
		if (header->fState == MemoryHeader::eUsed ) {	// most likely case first
			// recycle into pool
			PoolBucket *bucket = FindBucketBySize( header->fSize + MemoryHeader::AlignedSize());

			if ( bucket ) {
				Assert(header->fSize + MemoryHeader::AlignedSize() == bucket->fSize);

				MemTrackFree(fPoolTracker, header->fSize);
				InsertFreeHeaderIntoBucket(header, bucket);
			} else {
				SysLog::Error("bucket not found!");
				Assert(0);
			}
		} else if ( header->fState == MemoryHeader::eUsedNotPooled ) {
			MemTrackFree(fExcessTracker, header->fSize);
			::free(header);
		} else  {
			// something wrong happened, double free
			SysLog::Error("wrong header status, double free?");
			String strBuf(Storage::Global());
			strBuf << "MemoryHeader [";
			{
				String strContent((void *)header, (long)header->AlignedSize(), Storage::Global());
				OStringStream stream(strBuf);
				strContent.DumpAsHex(stream, (long)header->AlignedSize());
			}
			strBuf << "]";
			SysLog::Error(strBuf);
			strBuf.Trim(0L);
			strBuf << "Buffer, Size:" << (l_long)header->fSize << " [";
			{
				String strContent((void *)ExtMemStart(header), header->fSize, Storage::Global());
				OStringStream stream(strBuf);
				strContent.DumpAsHex(stream, 16L);
			}
			strBuf << "]";
			SysLog::Error(strBuf);
			Assert(0);
		}
	} else {
		// something wrong, either 0 ptr, invalid pointer or corrupted memory
		Assert(0 == vp);
	}
}

// users of sizeHint pay a price for optimal sizing...
size_t PoolAllocator::SizeHint(size_t size)
{
	PoolBucket *bucket = FindBucketBySize(size);
	if (bucket) {
		return bucket->fSize;
	} else {
		return size;
	}
}

void *PoolAllocator::Alloc(u_long allocSize)
{
	// allocSize includes the size of the necessary MemoryHeader!

	PoolBucket *bucket = FindBucketBySize(allocSize);
	// assume we found the order or can't fit it into order
	if ( bucket ) {
		if ( bucket->fFirstFree ) {
			// we have a bucket to reuse
			MemoryHeader *mh =  RemoveHeaderFromBucket(bucket) ;
			MemTrackAlloc(fPoolTracker, mh->fSize);
			return ExtMemStart(mh);
		} else {
			void *lastFree = fPoolBuckets[fNumOfPoolBucketSizes].fFirstFree;

			if ( ((char *)lastFree - (char *)fPoolMemory) > (long)bucket->fSize) {	// bucket size fits into long
				// we have enough free pool memory to satisfy the request
				MemoryHeader *mh = MakeHeaderFromBucket(bucket, lastFree);
				fPoolBuckets[fNumOfPoolBucketSizes].fFirstFree = mh;

				MemTrackAlloc(fPoolTracker, mh->fSize);
				return ExtMemStart( mh );
			} else {
				// try to split a larger bucket
				PoolBucket *requestedBucket = bucket;
				while ( bucket && !bucket->fFirstFree ) {
					bucket++;    // get next larger available bucket
				}

				if ( bucket && bucket->fFirstFree && bucket->fSize > 0 ) {
					// split larger bucket
					MemoryHeader *mh = RemoveHeaderFromBucket(bucket);

					long nbuckets = bucket->fSize / requestedBucket->fSize;
					char *buketEnd = ((char *)mh) + requestedBucket->fSize * nbuckets;
					Assert((char *)mh + mh->fSize + MemoryHeader::AlignedSize() == buketEnd);

					for (long i = 0; i < nbuckets; i++) {
						// create new buckets of requested size
						mh = MakeHeaderFromBucket(requestedBucket, buketEnd);
						buketEnd -= requestedBucket->fSize;
						InsertFreeHeaderIntoBucket(mh, requestedBucket);
					}
					// we have a bucket to reuse
					mh =  RemoveHeaderFromBucket(requestedBucket) ;

					MemTrackAlloc(fPoolTracker, mh->fSize);
					return ExtMemStart(mh);
				}
			}
		}
	}
	// last resort
	// we are out of pool memory at the moment
	// or have a larger piece of memory requested than largest pool bucket
	void *vp = ::calloc(allocSize, 1);
	if (vp) {
		// keep some statistic
		MemoryHeader *mh = new(vp) MemoryHeader(allocSize - MemoryHeader::AlignedSize(),
												MemoryHeader::eUsedNotPooled);
		MemTrackAlloc(fExcessTracker, mh->fSize);
		SysLog::WriteToStderr("|", 1); // visually mark pool overflow
		return ExtMemStart(mh);
	}
	static const char crashmsg[] = "FATAL: PoolAllocator::Alloc [global memory] calloc failed. I will crash :-(\n";
	SysLog::WriteToStderr(crashmsg, sizeof(crashmsg));

	return 0;
}

MemoryHeader *PoolAllocator::RemoveHeaderFromBucket(PoolBucket *bucket)
{
	MemoryHeader *mh = (MemoryHeader *)bucket->fFirstFree;
	Assert(mh->fMagic == MemoryHeader::gcMagic);
	mh->fState = MemoryHeader::eUsed;
	bucket->fFirstFree = mh->fNext;
	return mh;
}

MemoryHeader *PoolAllocator::MakeHeaderFromBucket(PoolBucket *bucket, void *lastFree)
{
	// create header from bucket
	lastFree = ((char *)lastFree - bucket->fSize);
	Assert(((char *)lastFree - (char *)fPoolMemory) >= 0 );
	MemoryHeader *mh = new(lastFree) MemoryHeader(bucket->fUsableSize, MemoryHeader::eUsed);
	return mh;
}

void PoolAllocator::InsertFreeHeaderIntoBucket(MemoryHeader *mh, PoolBucket *bucket)
{
	Assert(mh->fMagic == MemoryHeader::gcMagic);
	mh->fState = MemoryHeader::eFree;
	mh->fNext = (MemoryHeader *)bucket->fFirstFree;
	bucket->fFirstFree = mh;
}

PoolBucket *PoolAllocator::FindBucketBySize( u_long allocSize)
{
	// find smallest bucket suitable to store allocSize
	for (u_long order = 0; order < fNumOfPoolBucketSizes; order++) {
		if (fPoolBuckets[order].fSize >= allocSize ) { // yeah it fits
			return fPoolBuckets + order;
		}
	}
	return 0;
}

void PoolAllocator::PrintStatistic()
{
	MemTrackStatIfAllocated(fPoolTracker);
	MemTrackStatIfAllocated(fExcessTracker);
}

#ifdef MEM_DEBUG
l_long PoolAllocator::CurrentlyAllocated()
{
	return fPoolTracker.CurrentlyAllocated() + fExcessTracker.CurrentlyAllocated();
}
#endif

void PoolAllocator::Refresh()
{
	MemTrackStatIfAllocatedTriggered(PoolAllocator.Refresh, fPoolTracker);
	MemTrackStillAllocatedException(fPoolTracker, "PoolAllocator::Refresh()");
	MemTrackStatIfAllocatedTriggered(PoolAllocator.Refresh, fExcessTracker);
	MemTrackStillAllocatedException(fExcessTracker, "PoolAllocator::Refresh()");

	if ( !MemTrackStillAllocated(fPoolTracker) ) {
		// reinitialize pool if everything was freed
		Initialize();
	}
}
