/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include
#include "Storage.h"
#include "MemHeader.h"

//--- c-library modules used ---
#include <stdlib.h>

extern void MT_Storage__Initialize();
extern void MT_Storage__Finalize();
extern int MT_Storage__Free(void *vp);
extern Allocator *MT_Storage__Global();
extern Allocator *MT_Storage__Current();

#ifdef MEM_DEBUG
extern MemTracker *MT_Storage__GlobalTracker();

//------------- Utilities for Memory Tracking --------------
MemTracker::MemTracker() : fAllocated(0) , fNumAllocs(0), fSizeAllocated(0), fNumFrees(0), fSizeFreed(0) {}

MemTracker::~MemTracker() {}

void MemTracker::TrackAlloc(u_long allocSz)
{
	fAllocated += allocSz;
	fNumAllocs++;
	fSizeAllocated += allocSz;
}

void MemTracker::TrackFree(u_long allocSz)
{
	fAllocated -= allocSz;
	fNumFrees++;
	fSizeFreed += allocSz;
}

long long MemTracker::CurrentlyAllocated()
{
	return  fAllocated;
}

void MemTracker::PrintStatistic()
{
	cout << "Currently Allocated " << ":" << fAllocated << endl;
	cout << "Total Allocated " << fSizeAllocated << " bytes in " << fNumAllocs << " runs (" << fSizeAllocated / ((fNumAllocs) ? fNumAllocs : 1) << "/run)" << endl;
	cout << "Total Freed " << fSizeFreed << " bytes in " << fNumFrees << " runs (" << fSizeFreed / ((fNumFrees) ? fNumFrees : 1) << "/run)" << endl;
	cout << "Total " << (long long) fSizeAllocated - fSizeFreed << endl;
}
#endif

#ifdef __GNUG__
#include "new.h"
#else
void *operator new(size_t size, void *vp)
{
	Assert(size > 0);
	if (vp == 0) {
		return ::calloc(size, sizeof(char));
	}
	return vp;
}

void operator delete(void *ptr)
{
	if (ptr) {
		::free(ptr);
	}
}
#endif

//---- Storage ------------------------------------------
Allocator *Storage::fgGlobalPool = 0;

void Storage::Initialize()
{
	if ( !fgGlobalPool ) {
		atexit(Storage::Finalize);

#ifdef USE_MT_STORAGE
		MT_Storage__Initialize();
#endif
		void *vp = ::calloc(1, sizeof(GlobalAllocator));
		if ( vp ) {
			fgGlobalPool = new (vp) GlobalAllocator();
		} else {
			cerr << "Panic: allocation for global allocator failed" << endl;
			exit(1);
		}

#ifdef MEM_DEBUG
		fgGlobalPool->PrintStatistic();
#endif
	}
}

void Storage::Finalize()
{
#ifdef USE_MT_STORAGE
	MT_Storage__Finalize();
#endif
	if ( fgGlobalPool ) {
#ifdef MEM_DEBUG
		// terminate global allocator
		fgGlobalPool->PrintStatistic();
#endif
		fgGlobalPool->~Allocator();
		::free(fgGlobalPool);
	}
}

#ifdef MEM_DEBUG
void Storage::PrintStatistic()
{
	if ( fgGlobalPool ) {
		fgGlobalPool->PrintStatistic();
	}
}
#endif
//: same as malloc, prints statistics
void *Storage::Malloc(size_t size)
{
	return Current()->Malloc(size);
}

//: same as calloc
void *Storage::Calloc(int n, size_t size)
{
	return Current()->Calloc(n, size);
}

Allocator *Storage::Current()
{
#ifdef USE_MT_STORAGE
	return MT_Storage__Current();
#else
	return Storage::Global();
#endif
}

Allocator *Storage::Global()
{
	if ( !fgGlobalPool ) {
#ifdef USE_MT_STORAGE
		MT_Storage__Global();
#else
		Initialize();
#endif
	}
	return fgGlobalPool;
}

//: same as free, no statistics
void Storage::Free(void *vp)
{
#ifdef USE_MT_STORAGE
	MT_Storage__Free(vp);
#else
	Global()->Free(vp);
#endif
}

//--- Allocator -------------------------------------------------
Allocator::Allocator(long allocatorid) : fAllocatorId(allocatorid) { }
Allocator::~Allocator() { }

void *Allocator::Calloc(int n, size_t size)
{
	return Alloc(AllocSize(n, size));
}

void *Allocator::Malloc(size_t size)
{
	return Alloc(AllocSize(size, 1));
}

void Allocator::Refresh()
{
	// give the allocator opportunity to reorganize
}

u_long Allocator::AllocSize(u_long n, size_t size)
{
	return (n * size) + MemoryHeader::AlignedSize();
}

void *Allocator::ExtMemStart(void *vp)
{
	if (vp) {
		MemoryHeader *mh = (MemoryHeader *)vp;
		Assert(mh->fMagic == MemoryHeader::gcMagic);
		void *s = (((char *)(vp)) + MemoryHeader::AlignedSize()); // fSize does *not* include header
		memset(s, '\0', mh->fSize);
		return s;
	}
	return 0;
}

MemoryHeader *Allocator::RealMemStart(void *vp)
{
	if ( vp ) {
		MemoryHeader *mh = (MemoryHeader *) (((char *)(vp)) - MemoryHeader::AlignedSize());
		if (mh->fMagic == MemoryHeader::gcMagic) {
			return mh;
		}
	}
	return 0;
}

//---- GlobalAllocator ------------------------------------------
GlobalAllocator::GlobalAllocator() : Allocator(0L)
{
#ifdef MEM_DEBUG
#ifdef USE_MT_STORAGE
	fTracker = MT_Storage__GlobalTracker();
#else
	fTracker = new MemTracker();
#endif
#endif
}

GlobalAllocator::~GlobalAllocator()
{
#ifdef MEM_DEBUG
	if (fTracker) {
		delete fTracker;
	}
#endif
}

#ifdef MEM_DEBUG
void GlobalAllocator::PrintStatistic()
{
	MemTrackStat((*fTracker));
}

unsigned long long GlobalAllocator::CurrentlyAllocated()
{
	return fTracker->CurrentlyAllocated();
}
#endif

void *GlobalAllocator::Alloc(u_long allocSize)
{
	void *vp = ::calloc(allocSize, 1);
	if (vp) {
		MemoryHeader *mh = new(vp) MemoryHeader(allocSize - MemoryHeader::AlignedSize(),
												MemoryHeader::eUsedNotPooled);
		MemTrackAlloc((*fTracker), mh->fSize);

		return ExtMemStart(vp);
	}
	return NULL;
}

void  GlobalAllocator::Free(void *vp)
{
	if ( vp ) {
		MemoryHeader *header = RealMemStart(vp);
		if (header && header->fMagic == MemoryHeader::gcMagic) {
			Assert(header->fMagic == MemoryHeader::gcMagic); // should combine magic with state
			Assert(header->fState & MemoryHeader::eUsed);
			MemTrackFree((*fTracker), header->fSize);
			if ( header ) {
				::free(header);
			}
		} else {
			::free(vp); // we didn't get a pointer that we allocated
		}
	}
}

