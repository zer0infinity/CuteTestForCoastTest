/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "PoolAllocator.h"
#include "MemHeader.h"
#include "SystemLog.h"
#include "StringStream.h"
#include "Tracer.h"
#include "SystemBase.h"
#include <algorithm>
#include <cstring>

//! smallest size of allocation unit: 16UL for the usable memory block
static const size_t fgMinPayloadSize = 16UL;

struct PoolBucket {
	size_t fSize;
	size_t fUsableSize;
	void *fFirstFree;
	MemTracker *fpBucketTracker;
};

ExcessTrackerElt::ExcessTrackerElt()
	: fpTracker(NULL)
	, fpNext(NULL)
	, fulPayloadSize(0)
{
}

ExcessTrackerElt::ExcessTrackerElt(MemTracker *pTracker, ExcessTrackerElt *pNext, size_t ulPayloadSize)
	: fpTracker(pTracker)
	, fpNext(pNext)
	, fulPayloadSize(ulPayloadSize)
{
}

void ExcessTrackerElt::SetValues(MemTracker *pTracker, ExcessTrackerElt *pNext, size_t ulPayloadSize)
{
	fpTracker = pTracker;
	fpNext = pNext;
	fulPayloadSize = ulPayloadSize;
}

void *ExcessTrackerElt::operator new(size_t size)
{
	// never allocate on allocator, because we are tracking exactly the one...
	void *vp = ::calloc(1, sizeof(ExcessTrackerElt));
	return vp;
}

void *ExcessTrackerElt::operator new(size_t size, Allocator *)
{
	// never allocate on allocator, because we are tracking exactly the one...
	void *vp = ::calloc(1, sizeof(ExcessTrackerElt));
	return vp;
}

void ExcessTrackerElt::operator delete(void *vp)
{
	if (vp) {
		::operator delete(vp);
	}
}

ExcessTrackerElt::~ExcessTrackerElt()
{
	// recursively print statistics on excess trackers
	PrintStatistic(2);//lint !e1551
	delete fpTracker;
	fpTracker = 0;
	delete fpNext;//lint !e1551
	fpNext = 0;
}

void ExcessTrackerElt::PrintStatistic(long lLevel)
{
	if ( fpTracker && fpTracker->PeakAllocated() > 0 ) {
		fpTracker->PrintStatistic(lLevel);
	}
}

ul_long ExcessTrackerElt::GetSizeToPowerOf2(size_t ulWishSize)
{
	long lBitCnt = 0L, lMaxBit = 0L;
	while ( ulWishSize > 0 ) {
		++lMaxBit;
		// count bits to see if the wish size is already an exact power of 2
		lBitCnt += ( ulWishSize & 0x01 );
		ulWishSize >>= 1;
	}
	// adjust bitcount if wish size is already an exact power of 2
	if ( lBitCnt == 1 ) {
		--lMaxBit;
	}
	return (1 << lMaxBit);//lint !e647
}

long ExcessTrackerElt::GetLargestExcessEltBitNum()
{
	long lBitCnt = 0L;
	ExcessTrackerElt *pElt = this;
	while ( pElt->fpNext != NULL ) {
		pElt = pElt->fpNext;
	}
	if ( pElt ) {
		ul_long ulSize = pElt->fulPayloadSize;
		// count bit number
		while ( ulSize > 0 ) {
			ulSize >>= 1;
			++lBitCnt;
		}
	}
	return lBitCnt;
}

MemTracker *ExcessTrackerElt::FindTrackerForSize(size_t ulPayloadSize)
{
	MemTracker *pTracker = NULL;
	ExcessTrackerElt *pElt = this;
	ulPayloadSize = std::max( ulPayloadSize, fgMinPayloadSize );
	// the list of trackers is ordered ascending by size
	while ( pElt != NULL ) {
		// try if the this tracker is already of correct size
		if ( ulPayloadSize <= pElt->fulPayloadSize ) {
			size_t ulNextSmallerBucketPayloadSz = std::max( ( pElt->fulPayloadSize >> 1 ), fgMinPayloadSize );
			if ( ulPayloadSize > ulNextSmallerBucketPayloadSz ) {
				// payload fits into this trackers range, return the tracker
				pTracker = pElt->fpTracker;
			}
			break;
		}
		// not found yet
		pElt = pElt->fpNext;
	}
	return pTracker;
}

ExcessTrackerElt *ExcessTrackerElt::InsertTrackerForSize(MemTracker *pTracker, size_t ulPayloadSize)
{
	ExcessTrackerElt *pElt = this, *pSmaller = NULL;
	ulPayloadSize = std::max( ulPayloadSize, fgMinPayloadSize );
	while ( pElt && pElt->fulPayloadSize > 0 ) {
		if ( pElt->fulPayloadSize > ulPayloadSize ) {
			// current element is already larger, must insert between pSmaller and pElt
			break;
		}
		// not found yet
		pSmaller = pElt;
		pElt = pElt->fpNext;
	}
	// check the case where we get the root element
	if ( pSmaller == NULL && pElt != NULL ) {
		ExcessTrackerElt *pNewElt = NULL;
		// check if the root element must be shifted
		if ( pElt->fulPayloadSize > 0 ) {
			// use copy-ctor to assign values of root element to the shifted one
			pNewElt = new ExcessTrackerElt((*pElt));
		}
		// assign current values to root element using assignment operator and temporary object
		pElt->SetValues(pTracker, pNewElt, ulPayloadSize);
	} else {
		if ( pSmaller != NULL ) {
			pElt = pSmaller->fpNext = new ExcessTrackerElt(pTracker, pElt, ulPayloadSize);
		} else {
			// should not happen
		}
	}
	return pElt;
}

void ExcessTrackerElt::SetId(long lId)
{
	ExcessTrackerElt *pElt = this;
	while ( pElt ) {
		MemTracker *pTracker = pElt->fpTracker;
		if ( pTracker ) {
			pTracker->SetId(lId);
		}
		pElt = pElt->fpNext;
	}
}

MemTracker *ExcessTrackerElt::operator[](size_t ulPayloadSize)
{
	size_t ulWishSize = std::max<size_t>( GetSizeToPowerOf2(ulPayloadSize), fgMinPayloadSize);
	MemTracker *pTracker = FindTrackerForSize(ulWishSize);
	if ( pTracker == NULL ) {
		const int bufSize = 128;
		char buf[bufSize] = { 0 };
		coast::system::SnPrintf(buf, bufSize, "PoolExcessTracker[%zu]", ulWishSize);
		// need to add new ExcessTrackerElt and MemTracker for given size
		pTracker = coast::storage::MakeMemTracker(buf, false);
		InsertTrackerForSize(pTracker, ulWishSize);
	}
	return pTracker;
}

ul_long ExcessTrackerElt::CurrentlyAllocated()
{
	ul_long llTotal = 0LL;
	ExcessTrackerElt *pElt = this;
	while ( pElt ) {
		MemTracker *pTracker = pElt->fpTracker;
		if ( pTracker ) {
			llTotal += pTracker->CurrentlyAllocated();
		}
		pElt = pElt->fpNext;
	}
	return llTotal;
}

void ExcessTrackerElt::Refresh()
{
	ExcessTrackerElt *pElt = this;
	while ( pElt ) {
		MemTracker *pTracker = pElt->fpTracker;
		if ( pTracker && ( pTracker->CurrentlyAllocated() > 0 ) ) {
			const int bufSize = 256;
			char buf[bufSize] = { 0 };
			coast::system::SnPrintf(buf, bufSize, "ExcessAllocator was still in use! (id: %ld, name: %s) in Refresh()", pTracker->GetId(), NotNull(pTracker->GetName()));//lint !e666
			SystemLog::Error(buf);
		}
		pElt = pElt->fpNext;
	}
}

PoolAllocator::PoolAllocator(long poolid, size_t poolSize, size_t maxPoolBuckets)
	: Allocator(poolid)
	, fNumOfPoolBucketSizes(maxPoolBuckets)
	, fpPoolTotalExcessTracker(NULL)
	, fpExcessTrackerList(NULL)
{
	StatTrace(PoolAllocator.PoolAllocator, "initializing PoolAllocator with id:" << poolid, coast::storage::Current());
	switch ( coast::storage::GetStatisticLevel() ) {
			// dummy entry, just in case the level gets extended
		case 4:
			// enable tracking of non-freed blocks
		case 3:
			// detailed statistics requested, where we also want to know how much excess memory was used
		case 2:
			fpExcessTrackerList = new ExcessTrackerElt();

			// overall statistics requested
		case 1:
			fTracker = Allocator::MemTrackerPtr(coast::storage::MakeMemTracker("PoolTotal", false));
			fpPoolTotalExcessTracker = coast::storage::MakeMemTracker("ExcessTotal", false);
			fTracker->SetId(poolid);
			fpPoolTotalExcessTracker->SetId(poolid);
	}

	fAllocSz = poolSize * 1024;
	fPoolMemory = ::calloc(fAllocSz, 1);
	fPoolBuckets = (PoolBucket *)::calloc(fNumOfPoolBucketSizes + 1, sizeof(PoolBucket));

	if (!fPoolMemory || !fPoolBuckets) {
		static const char crashmsg[] = "FATAL: PoolAllocator::PoolAllocator calloc failed. I will crash :-(\n";
		SystemLog::WriteToStderr(crashmsg, sizeof(crashmsg));

		// We might never see the following because we have no memory.
		String msg("PoolAllocator: ");
		msg << "allocation of PoolStorage: " << static_cast<long>(poolSize) << ", " << static_cast<long>(maxPoolBuckets) << " failed";
		SystemLog::Error(msg);
		PoolAllocator::Unref(); // signal allocation failure
		return;
	}
	Initialize();
}

long PoolAllocator::SetId(long lId)
{
	StatTrace(PoolAllocator.SetId, "setting id from fAllocatorId:" << fAllocatorId << " to:" << lId, coast::storage::Current());
	for (long i = 0; i < static_cast<long>(fNumOfPoolBucketSizes); ++i) {
		MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
		if ( pTracker ) {
			pTracker->SetId(lId);
		}
	}
	if ( fpExcessTrackerList ) {
		fpExcessTrackerList->SetId(lId);
	}
	fTracker->SetId(lId);
	if ( fpPoolTotalExcessTracker ) {
		fpPoolTotalExcessTracker->SetId(lId);
	}
	return Allocator::SetId(lId);
}

void PoolAllocator::Initialize()
{
	// initialize data structures used by the allocator
	long sz = fgMinPayloadSize;
	const size_t alignedSize = coast::memory::AlignedSize<MemoryHeader>::value;
	for (long i = 0; i < static_cast<long>(fNumOfPoolBucketSizes); ++i) {
		fPoolBuckets[i].fSize = sz + alignedSize;
		fPoolBuckets[i].fUsableSize = sz;
		fPoolBuckets[i].fFirstFree = NULL;
		// only create new trackers once
		if ( fPoolBuckets[i].fpBucketTracker == NULL && coast::storage::GetStatisticLevel() >= 2 ) {
			const int bufSize = 128;
			char buf[bufSize] = { 0 };
			coast::system::SnPrintf(buf, bufSize, "PoolBucketTracker[%ld]", sz);
			fPoolBuckets[i].fpBucketTracker = coast::storage::MakeMemTracker(buf, false);
			fPoolBuckets[i].fpBucketTracker->SetId(fAllocatorId);
		}
		sz <<= 1;
	}
	fPoolBuckets[fNumOfPoolBucketSizes].fSize = 0;
	fPoolBuckets[fNumOfPoolBucketSizes].fFirstFree = ((char *)fPoolMemory) + fAllocSz;
}

PoolAllocator::~PoolAllocator()
{
	String strUsedPoolSize("Pool usage: ", coast::storage::Global()), strUnusedBucketSizes("Unused bucket sizes: [", coast::storage::Global()), strUsedBucketSizes("Used bucket sizes:   [", coast::storage::Global());
	long lNumUsed = 0, lNumUnused = 0, lMaxUsedBucket = 0, lMaxExcessBit = 0;
	long lStatisticLevel = coast::storage::GetStatisticLevel();
	bool bFirst = true;
	// override user settings if excess memory was used
	if ( fpPoolTotalExcessTracker && fpPoolTotalExcessTracker->PeakAllocated() > 0 ) {
		lStatisticLevel = 2;
	}

	for (long i = 0; i < static_cast<long>(fNumOfPoolBucketSizes); ++i) {
		MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
		if ( pTracker ) {
			if ( pTracker->PeakAllocated() > 0 ) {
				pTracker->PrintStatistic(lStatisticLevel);
				if ( pTracker->CurrentlyAllocated() > 0 ) {
					if ( bFirst ) {
						const int bufSize = 256;
						char buf[bufSize] = { 0 };
						coast::system::SnPrintf(buf, bufSize, "PoolAllocator was still in use! (id: %ld, name: %s) in PoolAllocator::~PoolAllocator()", pTracker->GetId(), NotNull(pTracker->GetName()));//lint !e666
						SystemLog::Error(buf);
						bFirst = false;
					}
					IntDumpStillAllocated(pTracker, fPoolBuckets[i].fSize, fPoolBuckets[i].fUsableSize);
				}
				if ( ++lNumUsed > 1 ) {
					strUsedBucketSizes.Append(", ");
				}
				strUsedBucketSizes.Append(static_cast<u_long>(fPoolBuckets[i].fUsableSize));
				lMaxUsedBucket = i + 1;
			} else {
				if ( ++lNumUnused > 1 ) {
					strUnusedBucketSizes.Append(", ");
				}
				strUnusedBucketSizes.Append(static_cast<u_long>(fPoolBuckets[i].fUsableSize));
			}
		}
	}
	// will print statistics if list contains entries
	if ( fpExcessTrackerList ) {
		lMaxExcessBit = fpExcessTrackerList->GetLargestExcessEltBitNum();
		fpExcessTrackerList->SetId(fAllocatorId);
		delete fpExcessTrackerList;
		fpExcessTrackerList = NULL;
	}
	StatTrace(PoolAllocator.~PoolAllocator, "id:" << fAllocatorId << " ExcessTrackerList deleted", coast::storage::Global());
	if ( lStatisticLevel >= 1 ) {
		// totals
		if ( fTracker->PeakAllocated() > 0 ) {
			fTracker->PrintStatistic(2);
		}
		if ( fpPoolTotalExcessTracker && fpPoolTotalExcessTracker->PeakAllocated() > 0 ) {
			fpPoolTotalExcessTracker->PrintStatistic(2);
		}
	}
	strUsedPoolSize << "configured " << (l_long)( fAllocSz / 1024UL ) << "kB";
	if ( fTracker->PeakAllocated() > 0 ) {
		strUsedPoolSize << ", used " << (l_long)( fTracker->PeakAllocated() / 1024UL ) << "kB";
	}
	if ( fpPoolTotalExcessTracker && fpPoolTotalExcessTracker->PeakAllocated() > 0 ) {
		strUsedPoolSize << " excess " << (l_long)( fpPoolTotalExcessTracker->PeakAllocated() / 1024UL ) << "kB";
	}
	delete fpPoolTotalExcessTracker;
	fpPoolTotalExcessTracker = NULL;

	StatTrace(PoolAllocator.~PoolAllocator, "id:" << fAllocatorId << " PoolTotalTrackers deleted", coast::storage::Global());

	if ( lStatisticLevel >= 1 ) {
		SystemLog::WriteToStderr(String("\nAllocator [", coast::storage::Global()).Append(fAllocatorId).Append("]\n"));
		SystemLog::WriteToStderr(strUsedPoolSize.Append("\n"));
		if ( lNumUsed > 0  ) {
			strUsedBucketSizes.Append("]");
			ul_long ulSize = fgMinPayloadSize;
			while ( ulSize > 1 ) {
				ulSize >>= 1;
				--lMaxExcessBit;
			}
			lMaxUsedBucket = std::max(lMaxUsedBucket, lMaxExcessBit);
			strUnusedBucketSizes.Append("]\n -> optimal BucketSizesParam: ").Append(lMaxUsedBucket).Append(" now: ").Append(static_cast<u_long>(fNumOfPoolBucketSizes)).Append("\n");
			SystemLog::WriteToStderr(strUsedBucketSizes.Append('\n'));
			SystemLog::WriteToStderr(strUnusedBucketSizes);
		}
	}
	StatTrace(PoolAllocator.~PoolAllocator, "id:" << fAllocatorId << " deleting BucketTrackers", coast::storage::Global());
	for (long i = 0; i < static_cast<long>(fNumOfPoolBucketSizes); ++i) {
		delete (fPoolBuckets[i].fpBucketTracker);
	}
	StatTrace(PoolAllocator.~PoolAllocator, "id:" << fAllocatorId << " deleting PoolBuckets and PoolMemory", coast::storage::Global());
	::free(fPoolBuckets);
	::free(fPoolMemory);
}//lint !e1579

void PoolAllocator::DumpStillAllocated()
{
	bool bWasInUse = false;
	long i = 0;

	for (i = 0; i < static_cast<long>(fNumOfPoolBucketSizes); ++i) {
		MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
		if ( pTracker && pTracker->PeakAllocated() > 0 ) {
			if ( pTracker->CurrentlyAllocated() > 0 ) {
				bWasInUse = true;
				break;
			}
		}
	}
	if ( bWasInUse ) {
		bool bFirst = true;
		for (i = 0; i < static_cast<long>(fNumOfPoolBucketSizes); ++i) {
			MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
			if ( pTracker && ( pTracker->PeakAllocated() > 0 ) && ( pTracker->CurrentlyAllocated() > 0 ) ) {
				if ( bFirst ) {
					const int bufSize = 256;
					char buf[bufSize] = { 0 };
					coast::system::SnPrintf(buf, bufSize, "PoolAllocator was still in use! (id: %ld, name: %s) in PoolAllocator::DumpStillAllocated()", pTracker->GetId(), NotNull(pTracker->GetName()));//lint !e666
					SystemLog::Error(buf);
					bFirst = false;
				}
				IntDumpStillAllocated(pTracker, fPoolBuckets[i].fSize, fPoolBuckets[i].fUsableSize);
			}
		}
	}
}

void PoolAllocator::IntDumpStillAllocated(MemTracker *pTracker, size_t lSize, size_t lUsableSize)
{
	// finding unfreed items
	if ( pTracker ) {
		pTracker->DumpUsedBlocks();
	}
}

void PoolAllocator::Free(void* p, size_t sz) {
	Free(p);
}

void PoolAllocator::Free(void *vp)
{
	MemoryHeader *header = RealMemStart(vp);
	if ( header ) {
		if (header->fState == MemoryHeader::eUsed ) {	// most likely case first
			// recycle into pool
			const size_t alignedSize = coast::memory::AlignedSize<MemoryHeader>::value;
			PoolBucket *bucket = FindBucketBySize( header->fUsableSize + alignedSize);

			if ( bucket ) {
				Assert(header->fUsableSize + alignedSize == bucket->fSize);
				// as detail tracking (level 2) only takes place when total tracking (level 1) is enabled
				// the bucket tracker gets only checked when total tracking is also enabled
				fTracker->TrackFree(header);
				if ( bucket->fpBucketTracker ) {
					bucket->fpBucketTracker->TrackFree(header);
				}
				InsertFreeHeaderIntoBucket(header, bucket);
			} else {
				SystemLog::Error("bucket not found!");
				Assert(0);
			}
		} else if ( header->fState == MemoryHeader::eUsedNotPooled ) {
			// as excess tracking (level 2) only takes place when total tracking (level 1) is enabled
			// the excess tracker gets only checked when total tracking is also enabled
			if ( fpPoolTotalExcessTracker ) {
				fpPoolTotalExcessTracker->TrackFree(header);
				if ( fpExcessTrackerList ) {
					(*fpExcessTrackerList)[header->fUsableSize]->TrackFree(header);
				}
			}
			::free(header);
		} else  {
			// something wrong happened, double free
			SystemLog::Error("wrong header status, double free?");
			String strBuf(coast::storage::Global());
			strBuf << "MemoryHeader [";
			{
				const size_t alignedSize = coast::memory::AlignedSize<MemoryHeader>::value;
				String strContent((void *)header, alignedSize, coast::storage::Global());
				OStringStream stream(strBuf);
				strContent.DumpAsHex(stream, alignedSize);
			}
			strBuf << "]";
			SystemLog::Error(strBuf);
			strBuf.Trim(0L);
			strBuf << "Buffer, Size:" << (l_long)header->fUsableSize << " [";
			{
				String strContent((void *)ExtMemStart(header), header->fUsableSize, coast::storage::Global());
				OStringStream stream(strBuf);
				strContent.DumpAsHex(stream, 16L);
			}
			strBuf << "]";
			SystemLog::Error(strBuf);
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
	// as FindBucketBySize() internally compares against the given size against the (payload+MemoryHeader)
	//  we must take care of that here
	PoolBucket *bucket = FindBucketBySize(size + coast::memory::AlignedSize<MemoryHeader>::value);
	// if a bucket is found, we need to return fUsableSize and not fSize as it was before!
	// -> otherwise, the caller would finally allocate a larger block than needed
	if (bucket) {
		return bucket->fUsableSize;
	} else {
		return size;
	}
}

void *PoolAllocator::Alloc(size_t allocSize)
{
	// allocSize includes the size of the necessary MemoryHeader!

	PoolBucket *bucket = FindBucketBySize(allocSize);
	// assume we found the order or can't fit it into order
	if ( bucket ) {
		if ( bucket->fFirstFree ) {
			// we have a bucket to reuse
			MemoryHeader *mh = RemoveHeaderFromBucket(bucket);
			// as detail tracking (level 2) only takes place when total tracking (level 1) is enabled
			// the bucket tracker gets only checked when total tracking is also enabled
			fTracker->TrackAlloc(mh);
			if ( bucket->fpBucketTracker ) {
				bucket->fpBucketTracker->TrackAlloc(mh);
			}
			return ExtMemStart(mh);
		} else {
			void *lastFree = fPoolBuckets[fNumOfPoolBucketSizes].fFirstFree;

			if ( ((char *)lastFree - (char *)fPoolMemory) > (long)bucket->fSize) {	// bucket size fits into long
				// we have enough free pool memory to satisfy the request
				MemoryHeader *mh = MakeHeaderFromBucket(bucket, lastFree);
				fPoolBuckets[fNumOfPoolBucketSizes].fFirstFree = mh;
				// as detail tracking (level 2) only takes place when total tracking (level 1) is enabled
				// the bucket tracker gets only checked when total tracking is also enabled
				fTracker->TrackAlloc(mh);
				if ( bucket->fpBucketTracker ) {
					bucket->fpBucketTracker->TrackAlloc(mh);
				}
				return ExtMemStart( mh );
			} else {
				// try to split a larger bucket
				PoolBucket *requestedBucket = bucket;
				while ( bucket && !bucket->fFirstFree ) {
					++bucket; 		// get next larger available bucket
				}

				if ( bucket && bucket->fFirstFree && bucket->fSize > 0 ) {
					// split larger bucket
					MemoryHeader *mh = RemoveHeaderFromBucket(bucket);

					long nbuckets = bucket->fSize / requestedBucket->fSize;
					char *buketEnd = ((char *)mh) + requestedBucket->fSize * nbuckets;
// The following line does not apply anymore because only the payload size gets doubled for
// the next larger bucket but not the whole size consisting of payload plus header.
// This is why splitting a larger bucket into smaller ones would only fit by chance.
// -> As this can be avoided by specifying enough poolmemory, the following will only happen in rare situations.
//					Assert((char*)mh + mh->fUsableSize + coast::memory::AlignedSize<MemoryHeader>::value == buketEnd);

					for (long i = 0; i < nbuckets; ++i) {
						// create new buckets of requested size
						mh = MakeHeaderFromBucket(requestedBucket, buketEnd);
						buketEnd -= requestedBucket->fSize;
						InsertFreeHeaderIntoBucket(mh, requestedBucket);
					}
					// we have a bucket to reuse
					mh = RemoveHeaderFromBucket(requestedBucket) ;
					// as detail tracking (level 2) only takes place when total tracking (level 1) is enabled
					// the bucket tracker gets only checked when total tracking is also enabled
					fTracker->TrackAlloc(mh);
					if ( requestedBucket->fpBucketTracker ) {
						requestedBucket->fpBucketTracker->TrackAlloc(mh);
					}
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
		const size_t alignedSize = allocSize - coast::memory::AlignedSize<MemoryHeader>::value;
		MemoryHeader *mh = new (vp) MemoryHeader(alignedSize, MemoryHeader::eUsedNotPooled);
		// as excess tracking (level 2) only takes place when total tracking (level 1) is enabled
		// the excess tracker gets only checked when total tracking is also enabled
		if ( fpPoolTotalExcessTracker ) {
			fpPoolTotalExcessTracker->TrackAlloc(mh);
			if ( fpExcessTrackerList ) {
				(*fpExcessTrackerList)[alignedSize]->TrackAlloc(mh);
			}
		}
		return ExtMemStart(mh);//lint !e429
	}
	const int bufSize = 256;
	static char crashmsg[bufSize] = { 0 };
	coast::system::SnPrintf(crashmsg, bufSize, "FATAL: PoolAllocator::Alloc [global memory] calloc of sz:%zub failed. I will crash :-(\n", allocSize);
	SystemLog::WriteToStderr(crashmsg, -1);

	return 0;
}

MemoryHeader *PoolAllocator::RemoveHeaderFromBucket(PoolBucket *bucket)
{
	MemoryHeader *mh = (MemoryHeader *)bucket->fFirstFree;
	Assert(mh->fMagic == MemoryHeader::gcMagic);
	mh->fState = MemoryHeader::eUsed;
	bucket->fFirstFree = mh->fNextFree;
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
	mh->fNextFree = (MemoryHeader *)bucket->fFirstFree;
	bucket->fFirstFree = mh;
}

PoolBucket *PoolAllocator::FindBucketBySize( size_t allocSize)
{
	// find smallest bucket suitable to store allocSize
	for (size_t order = 0; order < fNumOfPoolBucketSizes; ++order) {
		if (fPoolBuckets[order].fSize >= allocSize ) {
			// yeah it fits
			return fPoolBuckets + order;
		}
	}
	return 0;
}

void PoolAllocator::PrintStatistic(long lLevel)
{
	long lStatisticLevel = ( ( lLevel >= 0 ) ? lLevel : coast::storage::GetStatisticLevel() );
	// override user setting if excess memory was used
	if ( fpPoolTotalExcessTracker && fpPoolTotalExcessTracker->PeakAllocated() > 0 ) {
		lStatisticLevel = 2;
	}

	for (long i = 0; i < static_cast<long>(fNumOfPoolBucketSizes); ++i) {
		MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
		if ( pTracker && pTracker->PeakAllocated() > 0 ) {
			pTracker->PrintStatistic(lStatisticLevel);
		}
	}
	if ( fpExcessTrackerList ) {
		fpExcessTrackerList->SetId(fAllocatorId);
		fpExcessTrackerList->PrintStatistic(lStatisticLevel);
	}

	if ( lStatisticLevel >= 1 ) {
		// totals
		if ( fTracker->PeakAllocated() > 0 ) {
			fTracker->PrintStatistic(2);
		}
		if ( fpPoolTotalExcessTracker && fpPoolTotalExcessTracker->PeakAllocated() > 0 ) {
			fpPoolTotalExcessTracker->PrintStatistic(2);
		}
	}
}

ul_long PoolAllocator::CurrentlyAllocated()
{
	ul_long llTotal = 0LL;
	llTotal += fTracker->CurrentlyAllocated();
	if ( fpPoolTotalExcessTracker ) {
		llTotal += fpPoolTotalExcessTracker->CurrentlyAllocated();
	}
	return llTotal;
}

void PoolAllocator::Refresh()
{
	if ( TriggerEnabled(PoolAllocator.Refresh) ) {//lint !e506//lint !e774
		PrintStatistic();
	}
	for (long i = 0; i < static_cast<long>(fNumOfPoolBucketSizes); ++i) {
		MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
		if ( pTracker && pTracker->CurrentlyAllocated() > 0 ) {
			const int bufSize = 256;
			char buf[bufSize] = { 0 };
			coast::system::SnPrintf(buf, bufSize, "PoolAllocator was still in use! (id: %ld, name: %s) in PoolAllocator::Refresh()", pTracker->GetId(), NotNull(pTracker->GetName()));//lint !e666
			SystemLog::Error(buf);
		}
	}
	if ( fpExcessTrackerList ) {
		fpExcessTrackerList->Refresh();
	}
	if ( CurrentlyAllocated() == 0 ) {
		// reinitialize pool if everything was freed
		Initialize();
	}
}
