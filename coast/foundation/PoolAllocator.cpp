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
#include <stdio.h>
#include <stdlib.h>

#if !defined(__SUNPRO_CC) ||  __SUNPRO_CC < 0x500
extern void *operator new(size_t size, void *vp);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
extern void operator delete(void *ptr, void *vp);
#endif
extern void operator delete(void *ptr);
#endif

//! smallest size of allocation unit
static const u_long fgStartSize = 16L;

//---- PoolAllocator ------------------------------------------
struct PoolBucket {
	u_long fSize;
	u_long fUsableSize;
	void *fFirstFree;
	MemTracker *fpBucketTracker;
};

ExcessTrackerElt::ExcessTrackerElt()
	: fpTracker(NULL)
	, fpNext(NULL)
	, fulBucketSize(0)
{
}

ExcessTrackerElt::ExcessTrackerElt(MemTracker *pTracker, ExcessTrackerElt *pNext, u_long ulBucketSize)
	: fpTracker(pTracker)
	, fpNext(pNext)
	, fulBucketSize(ulBucketSize)
{
}

void ExcessTrackerElt::SetValues(MemTracker *pTracker, ExcessTrackerElt *pNext, u_long ulBucketSize)
{
	fpTracker = pTracker;
	fpNext = pNext;
	fulBucketSize = ulBucketSize;
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
	PrintStatistic(2);
	delete fpTracker;
	fpTracker = NULL;
	delete fpNext;
	fpNext = NULL;
}

void ExcessTrackerElt::PrintStatistic(long lLevel)
{
	if ( fpTracker && fpTracker->PeakAllocated() > 0 ) {
		fpTracker->PrintStatistic(lLevel);
	}
}

ul_long ExcessTrackerElt::GetSizeToPowerOf2(u_long ulWishSize)
{
	long lBitCnt = 0L, lMaxBit = 0L;
	while ( ulWishSize > 0 ) {
		++lMaxBit;
		// count bits to see if the wish size is already an exact power of 2
		lBitCnt += ( ( ( ulWishSize & 0x01 ) > 0 ) ? 1 : 0 );
		ulWishSize >>= 1;
	}
	// adjust bitcount if wish size is already an exact power of 2
	if ( lBitCnt == 1 ) {
		--lMaxBit;
	}
	return (1 << lMaxBit);
}

long ExcessTrackerElt::GetMaxSizeBitNumber()
{
	long lBitCnt = 0L;
	ExcessTrackerElt *pElt = this;
	while ( pElt->fpNext != NULL ) {
		pElt = pElt->fpNext;
	}
	if ( pElt ) {
		ul_long ulSize = pElt->fulBucketSize;
		// count bit number
		while ( ulSize > 0 ) {
			ulSize >>= 1;
			++lBitCnt;
		}
	}
	return lBitCnt;
}

MemTracker *ExcessTrackerElt::FindTrackerForSize(u_long lMemSize)
{
	MemTracker *pTracker = NULL;
	ExcessTrackerElt *pElt = this;
	while ( pElt != NULL ) {
		// try if the this tracker is already of correct size
		// if the tracker is for a size which is too big, we should not take it
		// ( trackersize >> 1 ) > lMemSize <= trackersize
		if ( ( pElt->fulBucketSize >= lMemSize ) && ( lMemSize > ( pElt->fulBucketSize >> 1 ) ) ) {
			pTracker = pElt->fpTracker;
			// found, return the tracker
			break;
		}
		// not found yet
		pElt = pElt->fpNext;
	}
	return pTracker;
}

ExcessTrackerElt *ExcessTrackerElt::InsertTrackerForSize(MemTracker *pTracker, u_long lMemSize)
{
	ExcessTrackerElt *pElt = this, *pSmaller = NULL;
	while ( pElt && pElt->fulBucketSize > 0 ) {
		if ( pElt->fulBucketSize > lMemSize ) {
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
		if ( pElt->fulBucketSize > 0 ) {
			// use copy-ctor to assign values of root element to the shifted one
			pNewElt = new ExcessTrackerElt((*pElt));
		}
		// assign current values to root element using assignment operator and temporary object
		pElt->SetValues(pTracker, pNewElt, lMemSize);
	} else {
		if ( pSmaller != NULL ) {
			pElt = pSmaller->fpNext = new ExcessTrackerElt(pTracker, pElt, lMemSize);
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

MemTracker *ExcessTrackerElt::operator[](u_long lMemSize)
{
	u_long ulWishSize = GetSizeToPowerOf2(lMemSize);
	MemTracker *pTracker = FindTrackerForSize(ulWishSize);
	if ( pTracker == NULL ) {
		char buf[80] = { 0 };
		snprintf(buf, sizeof(buf), "PoolExcessTracker[%d]", (long)ulWishSize);
		// need to add new ExcessTrackerElt and MemTracker for given size
		pTracker = Storage::MakeMemTracker(buf);
		InsertTrackerForSize(pTracker, ulWishSize);
	}
	return pTracker;
}

l_long ExcessTrackerElt::CurrentlyAllocated()
{
	l_long llTotal = 0LL;
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
			char buf[256] = { 0 };
			snprintf(buf, sizeof(buf), "ExcessAllocator was still in use! (id: %d, name: %s) in Refresh()", pTracker->fId, NotNull(pTracker->fpName));
			SysLog::Error(buf);
		}
		pElt = pElt->fpNext;
	}
}

PoolAllocator::PoolAllocator(long poolid, u_long poolSize, u_long maxPoolBuckets)
	: Allocator(poolid)
	, fNumOfPoolBucketSizes(maxPoolBuckets)
	, fpPoolTotalTracker( Storage::MakeMemTracker("PoolTotal") )
	, fpPoolTotalExcessTracker( Storage::MakeMemTracker("ExcessTotal") )
	, fpExcessTrackerList(NULL)
{
	fpExcessTrackerList = new ExcessTrackerElt();
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
	fpPoolTotalTracker->SetId(poolid);
	fpPoolTotalExcessTracker->SetId(poolid);
}

long PoolAllocator::SetId(long lId)
{
	for (long i = 0; i < (long)fNumOfPoolBucketSizes; ++i) {
		MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
		if ( pTracker ) {
			pTracker->SetId(lId);
		}
	}
	fpExcessTrackerList->SetId(lId);
	fpPoolTotalTracker->SetId(lId);
	fpPoolTotalExcessTracker->SetId(lId);
	return Allocator::SetId(lId);
}

void PoolAllocator::Initialize()
{
	// initialize data structures used by the allocator
	long sz = fgStartSize;

	for (long i = 0; i < (long)fNumOfPoolBucketSizes; ++i) {
		fPoolBuckets[i].fSize = sz;
		fPoolBuckets[i].fUsableSize = sz - MemoryHeader::AlignedSize();	// optimization
		fPoolBuckets[i].fFirstFree = NULL;
		// only create new trackers once
		if ( fPoolBuckets[i].fpBucketTracker == NULL ) {
			char buf[80] = { 0 };
			snprintf(buf, sizeof(buf), "PoolBucketTracker[%d]", sz);
			fPoolBuckets[i].fpBucketTracker = Storage::MakeMemTracker(buf);
			fPoolBuckets[i].fpBucketTracker->SetId(fAllocatorId);
		}
		sz <<= 1;
	}
	fPoolBuckets[fNumOfPoolBucketSizes].fSize = 0;
	fPoolBuckets[fNumOfPoolBucketSizes].fFirstFree = ((char *)fPoolMemory) + fAllocSz;
}

PoolAllocator::~PoolAllocator()
{
	String strUsedPoolSize("Pool usage: ", Storage::Global()), strUnusedBucketSizes("Unused bucket sizes: [", Storage::Global()), strUsedBucketSizes("Used bucket sizes:   [", Storage::Global());
	long lNumUsed = 0, lNumUnused = 0, lMaxUsedBucket = 0;
	long lIntLevel = Storage::fglStatisticLevel;

	// override user setting if excess memory was used
	if ( fpPoolTotalExcessTracker->PeakAllocated() > 0 ) {
		lIntLevel = 2;
	}

	for (long i = 0; i < (long)fNumOfPoolBucketSizes; ++i) {
		MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
		if ( pTracker->PeakAllocated() > 0 ) {
			pTracker->PrintStatistic(lIntLevel);
			if ( pTracker->CurrentlyAllocated() > 0 ) {
				char buf[256] = { 0 };
				snprintf(buf, sizeof(buf), "PoolAllocator was still in use! (id: %d, name: %s) in PoolAllocator::~PoolAllocator()", pTracker->fId, NotNull(pTracker->fpName));
				SysLog::Error(buf);
			}
			if ( ++lNumUsed > 1 ) {
				strUsedBucketSizes.Append(", ");
			}
			strUsedBucketSizes.Append(fPoolBuckets[i].fSize);
			lMaxUsedBucket = i + 1;
		} else {
			if ( ++lNumUnused > 1 ) {
				strUnusedBucketSizes.Append(", ");
			}
			strUnusedBucketSizes.Append(fPoolBuckets[i].fSize);
		}
	}
	// will print statistics if list contains entries
	long lMaxExcessBit = fpExcessTrackerList->GetMaxSizeBitNumber();
	fpExcessTrackerList->SetId(fAllocatorId);
	delete fpExcessTrackerList;

	if ( lIntLevel >= 1 ) {
		// totals
		if ( fpPoolTotalTracker->PeakAllocated() > 0 ) {
			fpPoolTotalTracker->PrintStatistic(2);
		}
		if ( fpPoolTotalExcessTracker->PeakAllocated() > 0 ) {
			fpPoolTotalExcessTracker->PrintStatistic(2);
		}
	}
	strUsedPoolSize << "configured " << (l_long)( fAllocSz / 1024UL ) << "kB, used " << (l_long)( fpPoolTotalTracker->PeakAllocated() / 1024UL ) << "kB";
	if ( fpPoolTotalExcessTracker->PeakAllocated() > 0 ) {
		strUsedPoolSize << " excess " << ( fpPoolTotalExcessTracker->PeakAllocated() / 1024UL ) << "kB";
	}
	delete fpPoolTotalTracker;
	delete fpPoolTotalExcessTracker;

	if ( lIntLevel >= 1 && lNumUsed > 0 ) {
		if ( lMaxExcessBit > 0 ) {
			ul_long ulSize = fgStartSize;
			while ( ulSize > 1 ) {
				ulSize >>= 1;
				--lMaxExcessBit;
			}
			lMaxUsedBucket = itoMAX(lMaxUsedBucket, lMaxExcessBit);
		}
		SysLog::WriteToStderr(String("\nAllocator [", Storage::Global()).Append(fAllocatorId).Append("]\n"));
		SysLog::WriteToStderr(strUsedPoolSize.Append("\n"));
		SysLog::WriteToStderr(strUsedBucketSizes.Append("] -> optimal BucketSizesParam: ").Append(lMaxUsedBucket).Append(" now: ").Append(fNumOfPoolBucketSizes).Append("\n"));
		SysLog::WriteToStderr(strUnusedBucketSizes.Append("]\n"));
	}
	for (long i = 0; i < (long)fNumOfPoolBucketSizes; ++i) {
		delete (fPoolBuckets[i].fpBucketTracker);
	}
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

				bucket->fpBucketTracker->TrackFree(header->fSize);
				fpPoolTotalTracker->TrackFree(header->fSize);
				InsertFreeHeaderIntoBucket(header, bucket);
			} else {
				SysLog::Error("bucket not found!");
				Assert(0);
			}
		} else if ( header->fState == MemoryHeader::eUsedNotPooled ) {
			u_long ulSize = header->fSize + MemoryHeader::AlignedSize();
			(*fpExcessTrackerList)[ulSize]->TrackFree(ulSize);
			fpPoolTotalExcessTracker->TrackFree(ulSize);
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
			MemoryHeader *mh = RemoveHeaderFromBucket(bucket);
			bucket->fpBucketTracker->TrackAlloc(mh->fSize);
			fpPoolTotalTracker->TrackAlloc(mh->fSize);
			return ExtMemStart(mh);
		} else {
			void *lastFree = fPoolBuckets[fNumOfPoolBucketSizes].fFirstFree;

			if ( ((char *)lastFree - (char *)fPoolMemory) > (long)bucket->fSize) {	// bucket size fits into long
				// we have enough free pool memory to satisfy the request
				MemoryHeader *mh = MakeHeaderFromBucket(bucket, lastFree);
				fPoolBuckets[fNumOfPoolBucketSizes].fFirstFree = mh;
				bucket->fpBucketTracker->TrackAlloc(mh->fSize);
				fpPoolTotalTracker->TrackAlloc(mh->fSize);
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
					Assert((char *)mh + mh->fSize + MemoryHeader::AlignedSize() == buketEnd);

					for (long i = 0; i < nbuckets; ++i) {
						// create new buckets of requested size
						mh = MakeHeaderFromBucket(requestedBucket, buketEnd);
						buketEnd -= requestedBucket->fSize;
						InsertFreeHeaderIntoBucket(mh, requestedBucket);
					}
					// we have a bucket to reuse
					mh =  RemoveHeaderFromBucket(requestedBucket) ;
					requestedBucket->fpBucketTracker->TrackAlloc(mh->fSize);
					fpPoolTotalTracker->TrackAlloc(mh->fSize);
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
		MemoryHeader *mh = new(vp) MemoryHeader(allocSize - MemoryHeader::AlignedSize(), MemoryHeader::eUsedNotPooled);
		(*fpExcessTrackerList)[allocSize]->TrackAlloc(allocSize);
		fpPoolTotalExcessTracker->TrackAlloc(allocSize);

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
	for (u_long order = 0; order < fNumOfPoolBucketSizes; ++order) {
		if (fPoolBuckets[order].fSize >= allocSize ) {
			// yeah it fits
			return fPoolBuckets + order;
		}
	}
	return 0;
}

void PoolAllocator::PrintStatistic(long lLevel)
{
	long lIntLevel = ( ( lLevel >= 0 ) ? lLevel : Storage::fglStatisticLevel );
	// override user setting if excess memory was used
	if ( fpPoolTotalExcessTracker->PeakAllocated() > 0 ) {
		lIntLevel = 2;
	}

	for (long i = 0; i < (long)fNumOfPoolBucketSizes; ++i) {
		MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
		if ( pTracker->PeakAllocated() > 0 ) {
			pTracker->PrintStatistic(lIntLevel);
		}
	}
	fpExcessTrackerList->SetId(fAllocatorId);
	fpExcessTrackerList->PrintStatistic(lIntLevel);

	if ( lIntLevel >= 1 ) {
		// totals
		if ( fpPoolTotalTracker->PeakAllocated() > 0 ) {
			fpPoolTotalTracker->PrintStatistic(2);
		}
		if ( fpPoolTotalExcessTracker->PeakAllocated() > 0 ) {
			fpPoolTotalExcessTracker->PrintStatistic(2);
		}
	}
}

l_long PoolAllocator::CurrentlyAllocated()
{
	l_long llTotal = 0LL;
	for (long i = 0; i < (long)fNumOfPoolBucketSizes; ++i) {
		MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
		llTotal += pTracker->CurrentlyAllocated();
	}
	llTotal += fpExcessTrackerList->CurrentlyAllocated();
	return llTotal;
}

void PoolAllocator::Refresh()
{
	if ( TraceTriggered(PoolAllocator.Refresh, Storage::Global()) ) {
		PrintStatistic();
	}
	for (long i = 0; i < (long)fNumOfPoolBucketSizes; ++i) {
		MemTracker *pTracker = fPoolBuckets[i].fpBucketTracker;
		if ( pTracker->CurrentlyAllocated() > 0 ) {
			char buf[256] = { 0 };
			snprintf(buf, sizeof(buf), "PoolAllocator was still in use! (id: %d, name: %s) in PoolAllocator::~PoolAllocator()", pTracker->fId, NotNull(pTracker->fpName));
			SysLog::Error(buf);
		}
	}
	fpExcessTrackerList->Refresh();
	if ( CurrentlyAllocated() == 0 ) {
		// reinitialize pool if everything was freed
		Initialize();
	}
}
