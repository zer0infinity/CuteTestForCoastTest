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
#include "System.h"
#include "MemHeader.h"
#include "InitFinisManagerFoundation.h"

//--- c-library modules used ---------------------------------------------------
#include <string.h>

#ifdef __370__
extern "C" void finalize();
#endif

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
	if ( delta > 0 ) {
		char msgbuf[1024] = {'\0'};
		if (message) {
			SysLog::WriteToStderr( message, strlen(message));
		}
		int bufsz = snprintf(msgbuf, sizeof(msgbuf), "\nMem Usage change by %.0f bytes in %s\n", (double)delta, fScope);
		SysLog::WriteToStderr( msgbuf, bufsz );
	}
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
	, fpName(strdup(name))	// copy string to be more flexible when using temporary param objects like Strings
{
}

void *MemTracker::operator new(size_t size)
{
	// never allocate on allocator, because we are tracking exactly the one...
	void *vp = ::calloc(1, sizeof(MemTracker));
	return vp;
}

void *MemTracker::operator new(size_t size, Allocator *)
{
	// never allocate on allocator, because we are tracking exactly the one...
	void *vp = ::calloc(1, sizeof(MemTracker));
	return vp;
}

void MemTracker::operator delete(void *vp)
{
	if (vp) {
		::operator delete(vp);
	}
}

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

MemTracker::~MemTracker()
{
	delete fpName;
}

void MemTracker::SetId(long id)
{
	fId = id;
}

void MemTracker::TrackAlloc(MemoryHeader *mh)
{
	fAllocated += mh->fUsableSize;
	++fNumAllocs;
	fSizeAllocated += mh->fUsableSize;
	fMaxAllocated = itoMAX(fMaxAllocated, fAllocated);
	if ( Storage::GetStatisticLevel() >= 3 ) {
		fUsedList.push_front(mh);
	}
}

void MemTracker::TrackFree(MemoryHeader *mh)
{
	fAllocated -= mh->fUsableSize;
	++fNumFrees;
	fSizeFreed += mh->fUsableSize;
	if ( Storage::GetStatisticLevel() >= 3 ) {
		UsedListType::iterator aUsedIterator;
		for ( aUsedIterator = fUsedList.begin(); aUsedIterator != fUsedList.end(); ++aUsedIterator ) {
			if ( *aUsedIterator == mh ) {
				fUsedList.erase(aUsedIterator);
				break;
			}
		}
	}
}

void MemTracker::DumpUsedBlocks()
{
	if ( fUsedList.size() ) {
		SysLog::Error(String(Storage::Global()).Append("memory blocks still in use for ").Append(fpName).Append(':'));
		UsedListType::const_iterator aUsedIterator;
		long lIdx = 0;
		for ( aUsedIterator = fUsedList.begin(); aUsedIterator != fUsedList.end(); ++lIdx, ++aUsedIterator) {
			MemoryHeader *pMH = *aUsedIterator;
			// reserve memory for the following text plus four times the buffer size for dumping as text
			String strOut(( 40L + ( ( pMH->fUsableSize + MemoryHeader::AlignedSize() ) * 4L ) ), Storage::Global());
			strOut.Append("Block ").Append(lIdx).Append('\n');
			strOut.Append("MemoryHeader:\n").Append(String((void *)pMH, MemoryHeader::AlignedSize(), Storage::Global()).DumpAsHex()).Append('\n');
			strOut.Append("Content:\n").Append(String((void *)((char *)pMH + MemoryHeader::AlignedSize()), pMH->fUsableSize, Storage::Global()).DumpAsHex()).Append('\n');
			SysLog::WriteToStderr(strOut);
		}
	}
}

void MemTracker::PrintStatistic(long lLevel)
{
	if ( lLevel >= 2 ) {
		char buf[2048] = { 0 }; // safety margin for bytes
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
				 fId, fpName,
				 fMaxAllocated,
				 fSizeAllocated , fNumAllocs, (long)(fSizeAllocated / ((fNumAllocs) ? fNumAllocs : 1)),
				 fSizeFreed, fNumFrees, (long)(fSizeFreed / ((fNumFrees) ? fNumFrees : 1)),
				 fAllocated
				);
		SysLog::WriteToStderr(buf, strlen(buf));
	}
}

MemTracker *Storage::DoMakeMemTracker(const char *name)
{
	return new MemTracker(name);
}

MemTracker *Storage::MakeMemTracker(const char *name, bool bThreadSafe)
{
	if  (fgHooks && !fgForceGlobal) {
		return fgHooks->MakeMemTracker(name, bThreadSafe);
	}
	return Storage::DoMakeMemTracker(name);
}

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
long Storage::fglStatisticLevel = 0L;

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
	if ( fgHooks && !fgForceGlobal ) {
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
		const char *pEnvVar = getenv("TRACE_STORAGE");
		long lLevel = ( ( pEnvVar != 0 ) ? atol(pEnvVar) : 0 );
		fglStatisticLevel = lLevel;
		once = false;
	}
}

void Storage::DoFinalize()
{
	// terminate global allocator and force printing statistics above level 1
	if ( GetStatisticLevel() >= 1 ) {
		Storage::DoGlobal()->PrintStatistic(2);
	}
}

void Storage::PrintStatistic(long lLevel)
{
	DoGlobal()->PrintStatistic(lLevel);
}

Allocator *Storage::DoGlobal()
{
	if ( !Storage::fgGlobalPool ) {
		Storage::Initialize();
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
Allocator::Allocator(long allocatorid)
	: fAllocatorId(allocatorid)
	, fRefCnt(0)
{
}

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
		void *s = (((char *)(vp)) + MemoryHeader::AlignedSize()); // fUsableSize does *not* include header
		// superfluous, Calloc takes care: memset(s, '\0', mh->fUsableSize);
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
GlobalAllocator::GlobalAllocator()
	: Allocator(11223344L)
	, fTracker(NULL)
{
	if ( Storage::GetStatisticLevel() >= 1 ) {
		fTracker = Storage::MakeMemTracker("GlobalAllocator", false);
		fTracker->SetId(fAllocatorId);
	}
}

GlobalAllocator::~GlobalAllocator()
{
	if ( fTracker ) {
		delete fTracker;
		fTracker = NULL;
	}
}

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
	if ( fTracker ) {
		return fTracker->CurrentlyAllocated();
	}
	return 0LL;
}

void GlobalAllocator::PrintStatistic(long lLevel)
{
	if ( fTracker ) {
		fTracker->PrintStatistic(lLevel);
	}
}

void *GlobalAllocator::Alloc(u_long allocSize)
{
	void *vp = ::malloc(allocSize);
	if (vp) {
		MemoryHeader *mh = new(vp) MemoryHeader(allocSize - MemoryHeader::AlignedSize(), MemoryHeader::eUsedNotPooled);
		if ( fTracker ) {
			fTracker->TrackAlloc(mh);
		}
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
			if ( fTracker ) {
				fTracker->TrackFree(header);
			}
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

MemTracker *TestStorageHooks::MakeMemTracker(const char *name, bool)
{
	return new MemTracker(name);
}
