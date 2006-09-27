/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ITOStorage.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "MemHeader.h"
#include "InitFinisManagerFoundation.h"

//--- c-library modules used ---------------------------------------------------
#include <string.h>
#if !defined(WIN32)
#include <unistd.h>
#endif

#ifdef __370__
extern "C" void finalize();
#endif

#ifdef MEM_DEBUG
#include <stdio.h>
#if defined(WIN32)
#define snprintf	_snprintf
#endif

MemChecker::MemChecker(const char *scope, Allocator *a)
	: fAllocator(a)
	, fSizeAllocated(fAllocator->CurrentlyAllocated())
	, fScope(scope)
{
}

MemChecker::~MemChecker()
{
	TraceDelta("MemChecker.~MemChecker: ");
}

void MemChecker::TraceDelta(const char *message)
{
	l_long delta = CheckDelta();
	// CAUTION: DO NOT PROGRAM AS FOLLOWS !!!
	// IT IS ONLY NECESSARY HERE, BECAUSE THIS CODE MIGHT BE EXECUTED DURING
	// atexit() WHEN PART OF THE C++ ENVIRONMENT IS ALREADY GONE (like cerr and cerr)
	// THIS CODE WILL ONLY BE INCLUDED IN DEBUG VERSIONS SO NO SAFETY CONCERN
	// FOR PRODUCTION CODE. Peter.
	char msgbuf[1024] = {'\0'};
	if (message) {
		SysLog::WriteToStderr( message, strlen(message));
	}
	int bufsz = snprintf(msgbuf, sizeof(msgbuf), "\nMem Usage change by %.0f bytes in %s\n", (double)delta, fScope);
	SysLog::WriteToStderr( msgbuf, bufsz );
}

l_long MemChecker::CheckDelta()
{
	return fAllocator->CurrentlyAllocated() - fSizeAllocated;
}

//------------- Utilities for Memory Tracking --------------
MemTracker::MemTracker(const char *name)
	: fAllocated(0)
	, fMaxAllocated(0)
	, fNumAllocs(0)
	, fSizeAllocated(0)
	, fNumFrees(0)
	, fSizeFreed(0)
	, fId(-1)
	, fpName(name)
{}

void MemTracker::Init(MemTracker *t)
{
	fAllocated = t->fAllocated;
	fNumAllocs = t->fNumAllocs;
	fSizeAllocated = t->fSizeAllocated;
	fNumFrees = t->fNumFrees;
	fSizeFreed = t->fSizeFreed;
	fId = t->fId;
	fMaxAllocated = t->fMaxAllocated;
}

MemTracker::~MemTracker() {}

void MemTracker::SetId(long id)
{
	fId = id;
}

void MemTracker::TrackAlloc(u_long allocSz)
{
	fAllocated += allocSz;
	++fNumAllocs;
	fSizeAllocated += allocSz;
	fMaxAllocated = itoMAX(fMaxAllocated, fAllocated);
}

void MemTracker::TrackFree(u_long allocSz)
{
	fAllocated -= allocSz;
	++fNumFrees;
	fSizeFreed += allocSz;
}

void MemTracker::PrintStatistic()
{
	// CAUTION: DO NOT PROGRAM AS FOLLOWS !!!
	// IT IS ONLY NECESSARY HERE, BECAUSE THIS CODE MIGHT BE EXECUTED DURING
	// atexit() WHEN PART OF THE C++ ENVIRONMENT IS ALREADY GONE (like cerr and cerr)
	// THIS CODE WILL ONLY BE INCLUDED IN DEBUG VERSIONS SO NO SAFETY CONCERN
	// FOR PRODUCTION CODE. Peter.
	char buf[2048] ; // safety margin for bytes
	snprintf(buf, sizeof(buf), "\nAllocator [%02ld] [%s]\n"
#if defined(WIN32)
			 "Peek Allocated  %20I64d bytes\n"
			 "Total Allocated %20I64d bytes in %15I64d runs (%ld/run)\n"
			 "Total Freed     %20I64d bytes in %15I64d runs (%ld/run)\n"
			 "------------------------------------------\n"
			 "Difference      %20I64d bytes\n",
#else
			 "Peek Allocated  %20lld bytes\n"
			 "Total Allocated %20lld bytes in %15lld runs (%ld/run)\n"
			 "Total Freed     %20lld bytes in %15lld runs (%ld/run)\n"
			 "------------------------------------------\n"
			 "Difference      %20lld bytes\n",
#endif
			 fId, fpName, fMaxAllocated,
			 fSizeAllocated , fNumAllocs, (long)(fSizeAllocated / ((fNumAllocs) ? fNumAllocs : 1)),
			 fSizeFreed, fNumFrees, (long)(fSizeFreed / ((fNumFrees) ? fNumFrees : 1)),
			 fAllocated
			);
	SysLog::WriteToStderr(buf, strlen(buf));
}

MemTracker *Storage::DoMakeMemTracker(const char *name)
{
	return new MemTracker(name);
}

MemTracker *Storage::MakeMemTracker(const char *name)
{
	if  (fgHooks && !fgForceGlobal) {
		return fgHooks->MakeMemTracker(name);
	}
	return Storage::DoMakeMemTracker(name);
}
#endif

#if (defined(__GNUG__)  && __GNUC__ < 3)|| defined (__370__)
#include <new.h>
#elif (defined(__GNUG__)  && __GNUC__ >=3) || (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x500) || (defined(WIN32) && defined(ONLY_STD_IOSTREAM))
#include <new>
#else
void *operator new(size_t size, void *vp)
{
	Assert(size > 0);
	if (vp == 0) {
		return ::calloc(size, sizeof(char));
	}
	return vp;
}
#endif

#if defined(WIN32) && (_MSC_VER >= 1200) && !defined(ONLY_STD_IOSTREAM)// VC6 or greater
void operator delete(void *ptr, void *vp)
{
	if (ptr) {
		::free(ptr);
	}
}
#endif

#if !defined (WIN32)
void operator delete(void *ptr)
{
	if (ptr) {
		::free(ptr);
	}
}
#endif

//---- Storage ------------------------------------------
Allocator *Storage::fgGlobalPool = 0;
StorageHooks *Storage::fgHooks = 0; // exchange this object when MT_Storage is used
bool Storage::fgForceGlobal = false;

class EXPORTDECL_FOUNDATION StorageInitializer : public InitFinisManagerFoundation
{
public:
	StorageInitializer(unsigned int uiPriority)
		: InitFinisManagerFoundation(uiPriority) {
		IFMTrace("StorageInitializer created\n");
	}
	~StorageInitializer()
	{}

	virtual void DoInit() {
		IFMTrace("Storage::Initialize\n");
		Storage::Initialize();
	}
	virtual void DoFinis() {
		IFMTrace("Storage::Finalize\n");
		Storage::Finalize();
	}
};

static StorageInitializer *psgStorageInitializer = new StorageInitializer(0);

Allocator *Storage::Current()
{
	if (fgHooks && !fgForceGlobal) {
		return fgHooks->Current();
	}
	return Storage::DoGlobal();
}

Allocator *Storage::Global()
{
	if (fgHooks && !fgForceGlobal) {
		return fgHooks->Global();
	}
	return Storage::DoGlobal();
}

void Storage::Initialize()
{
	Storage::DoInitialize();
	if (fgHooks && !fgForceGlobal) {
		fgHooks->Initialize();
	}
}

void Storage::Finalize()
{
	if (fgHooks && !fgForceGlobal) {
		fgHooks->Finalize();
	}
	Storage::DoFinalize();
}

void Storage::DoInitialize()
{
	static bool once = true;
	if (once) {
		once = false;
	}
}

void Storage::DoFinalize()
{
	// terminate global allocator
	PoolTrackStat(Storage::DoGlobal());
}

void Storage::PrintStatistic()
{
	PoolTrackStat(DoGlobal());
}

Allocator *Storage::DoGlobal()
{
	if (!Storage::fgGlobalPool) {
		Storage::fgGlobalPool = new GlobalAllocator();
	}
	return Storage::fgGlobalPool;
}

StorageHooks *Storage::SetHooks(StorageHooks *h)
{
	StorageHooks *pOldHook = fgHooks;
	if ( fgHooks ) {
		fgHooks->Finalize();
	}
	if ( h == NULL && fgHooks ) {
		fgHooks = fgHooks->GetOldHook();
		pOldHook = fgHooks;
	} else {
		fgHooks = h;
		if ( fgHooks != NULL ) {
			fgHooks->Initialize();
			fgHooks->SetOldHook(pOldHook);
		}
	}
	return pOldHook;
}

//--- Allocator -------------------------------------------------
Allocator::Allocator(long allocatorid) : fAllocatorId(allocatorid), fRefCnt(0) { }
Allocator::~Allocator()
{
	Assert(0 == fRefCnt);
}

void *Allocator::Calloc(int n, size_t size)
{
	void *ret = Alloc(AllocSize(n, size));
	if (ret && n * size > 0) {
		memset(ret, 0, n * size);
	}
	return ret;
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
		Assert(((MemoryHeader *)vp)->fMagic == MemoryHeader::gcMagic);
		void *s = (((char *)(vp)) + MemoryHeader::AlignedSize()); // fSize does *not* include header
		// superfluous, Calloc takes care: memset(s, '\0', mh->fSize);
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
GlobalAllocator::GlobalAllocator() : Allocator(11223344L)
{
#ifdef MEM_DEBUG
	fTracker = Storage::MakeMemTracker("GlobalAllocator");
	fTracker->SetId(fAllocatorId);
#endif
}

GlobalAllocator::~GlobalAllocator()
{
#ifdef MEM_DEBUG
	if ( fTracker ) {
		delete fTracker;
	}
#endif
}

#ifdef MEM_DEBUG
MemTracker *GlobalAllocator::ReplaceMemTracker(MemTracker *t)
{
	MemTracker *pOld = fTracker;
	if ( fTracker ) {
		fTracker = t;
	}
	return pOld;
}

l_long GlobalAllocator::CurrentlyAllocated()
{
	return fTracker->CurrentlyAllocated();
}
#endif

void GlobalAllocator::PrintStatistic()
{
	MemTrackStat((*fTracker));
}

void *GlobalAllocator::Alloc(u_long allocSize)
{
	void *vp = ::malloc(allocSize);
	if (vp) {
		MemoryHeader *mh = new(vp) MemoryHeader(allocSize - MemoryHeader::AlignedSize(), MemoryHeader::eUsedNotPooled);
		MemTrackAlloc((*fTracker), mh->fSize);
		return ExtMemStart(mh);
	} else {
		static const char crashmsg[] = "FATAL: GlobalAllocator::Alloc malloc failed. I will crash :-(\n";
		SysLog::WriteToStderr(crashmsg, sizeof(crashmsg));
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
			vp = header;
		}
		::free(vp);
	}
}

#ifdef __370__
// need a C-function as the linker cannot resolve C++ here
void finalize()
{
	Storage::Finalize();
}
#endif

Allocator *TestStorageHooks::Global()
{
	return Storage::DoGlobal();
}

Allocator *TestStorageHooks::Current()
{
	if (fAllocator) {
		return fAllocator;
	}

	return Storage::DoGlobal();
}

void TestStorageHooks::Initialize()
{
}

void TestStorageHooks::Finalize()
{
}

TestStorageHooks::TestStorageHooks(Allocator *wdallocator)
	: fAllocator(wdallocator)
	, fpOldHook(NULL)
{
	fpOldHook = Storage::SetHooks(this);
}

TestStorageHooks::~TestStorageHooks()
{
	StorageHooks *pHook = Storage::SetHooks(NULL);
	Assert( pHook == fpOldHook && "another Storage::SetHook() was called without restoring old Hook!");
}

#ifdef MEM_DEBUG
MemTracker *TestStorageHooks::MakeMemTracker(const char *name)
{
	return new MemTracker(name);
}
#endif
