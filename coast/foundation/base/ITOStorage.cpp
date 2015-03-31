/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ITOStorage.h"
#include "SystemLog.h"
#include "SystemBase.h"
#include "MemHeader.h"
#include "AllocatorNewDelete.h"
#include <cstring>
#include "InitFinisManager.h"


MemChecker::MemChecker(const char *scope, Allocator *a) :
		fAllocator(a), fSizeAllocated((a ? fAllocator->CurrentlyAllocated() : 0LL)), fScope(scope) {
}

MemChecker::~MemChecker() {
	TraceDelta("MemChecker.~MemChecker: ");
} //lint !e1579

void MemChecker::TraceDelta(const char *message) {
	l_long delta = CheckDelta();
	if (delta != 0) {
		if (message) {
			SystemLog::WriteToStderr(message, -1);
		}
		const int bufSize = 1024;
		char msgbuf[bufSize] = { 0 };
		int charsStoredOrRequired = coast::system::SnPrintf(msgbuf, bufSize, "\nMem Usage change by %.0f bytes in %s\nAllocator [%02ld]\n", (double) delta, fScope,
				(fAllocator ? fAllocator->GetId() : 0L));
		SystemLog::WriteToStderr(msgbuf, charsStoredOrRequired>=bufSize?-1:charsStoredOrRequired);
	}
}

l_long MemChecker::CheckDelta() {
	return (fAllocator ? (fAllocator->CurrentlyAllocated() - fSizeAllocated) : 0LL);
}

MemTracker::MemTracker(const char *name)
	: fAllocated(0)
	, fMaxAllocated(0)
	, fNumAllocs(0)
	, fSizeAllocated(0)
	, fNumFrees(0)
	, fSizeFreed(0)
	, fId(-1)
	, fpName(strdup(name))	// copy string to be more flexible when using temporary param objects like Strings
	, fpUsedList(NULL)
{
	if ( coast::storage::GetStatisticLevel() >= 3 ) {
		fpUsedList = new UsedListType();
	}
}

MemTracker::~MemTracker() {
	if (fpUsedList) {
		delete fpUsedList;	//lint !e1551
		fpUsedList = 0;
	}
	::free(fpName);
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
	fMaxAllocated = std::max(fMaxAllocated, fAllocated);
	// only track used pool memory buckets
	if ( fpUsedList && !( mh->fState & MemoryHeader::eNotPooled ) ) {
		fpUsedList->push_back(mh);
	}
}//lint !e429

void MemTracker::TrackFree(MemoryHeader *mh)
{
	fAllocated -= mh->fUsableSize;
	++fNumFrees;
	fSizeFreed += mh->fUsableSize;
	// only track used pool memory buckets
	if ( fpUsedList && !( mh->fState & MemoryHeader::eNotPooled ) ) {
		UsedListType::iterator aUsedIterator;
		for ( aUsedIterator = fpUsedList->begin(); aUsedIterator != fpUsedList->end(); ++aUsedIterator ) {
			if ( *aUsedIterator == mh ) {
				fpUsedList->erase(aUsedIterator);
				break;
			}
		}
	}
}

void MemTracker::DumpUsedBlocks()
{
	if ( fpUsedList && fpUsedList->size() ) {
		SystemLog::Error(String(coast::storage::Global()).Append("memory blocks still in use for ").Append(fpName).Append(':'));
		UsedListType::const_iterator aUsedIterator;
		const size_t alignedSize = coast::memory::AlignedSize<MemoryHeader>::value;
		long lIdx = 0;
		for ( aUsedIterator = fpUsedList->begin(); aUsedIterator != fpUsedList->end(); ++lIdx, ++aUsedIterator) {
			MemoryHeader *pMH = *aUsedIterator;
			// reserve memory for the following text plus four times the buffer size for dumping as text
			String strOut(( 40L + ( ( pMH->fUsableSize + alignedSize ) * 4L ) ), coast::storage::Global());
			strOut.Append("Block ").Append(lIdx).Append('\n');
			strOut.Append("MemoryHeader:\n").Append(String((void *)pMH, alignedSize, coast::storage::Global()).DumpAsHex()).Append('\n');
			strOut.Append("Content:\n").Append(String((void *)((char *)pMH + alignedSize), pMH->fUsableSize, coast::storage::Global()).DumpAsHex()).Append('\n');
			SystemLog::WriteToStderr(strOut);
		}
	}
}

void MemTracker::PrintStatistic(long lLevel)
{
	if ( lLevel >= 2 ) {
		const int bufSize = 2048;
		char buf[bufSize] = {0};
		coast::system::SnPrintf(buf, bufSize, "\nAllocator [%ld] [%s]\n"
#if defined(WIN32)
				 "Peek Allocated  %20I64d bytes\n"
				 "Total Allocated %20I64d bytes in %15I64d runs (%ld/run)\n"
				 "Total Freed     %20I64d bytes in %15I64d runs (%ld/run)\n"
				 "------------------------------------------\n"
				 "Difference      %20I64d bytes\n",
#else
				 "Peek Allocated  %20llu bytes\n"
				 "Total Allocated %20llu bytes in %20llu runs (%llu/run)\n"
				 "Total Freed     %20llu bytes in %20llu runs (%llu/run)\n"
				 "------------------------------------------\n"
				 "Difference      %20llu bytes\n",
#endif
				 fId, fpName,
				 fMaxAllocated,
				 fSizeAllocated , fNumAllocs, (fSizeAllocated / ((fNumAllocs) ? fNumAllocs : 1)),
				 fSizeFreed, fNumFrees, (fSizeFreed / ((fNumFrees) ? fNumFrees : 1)),
				 fAllocated
				);
		SystemLog::WriteToStderr(buf, -1);
	}
}

NullMemTracker::NullMemTracker(const char *) :
		MemTracker("NullMemTracker") {
}

NullMemTracker::~NullMemTracker() {
}

#include "singleton.hpp"
namespace {
	class StorageInitializer {
		/* define the logging level of memory statistics by defining COAST_TRACE_STORAGE appropriately
						0: No pool statistic tracing, even not for excess memory nor GlobalAllocator usage
						1: Trace overall statistics
						2: + trace detailed statistics
						3: + keep track of allocated blocks to trace them in case they were not freed */
		long statisticLevel;
	public:
		StorageInitializer() {
			const char *pEnvVar = getenv("COAST_TRACE_STORAGE");
			long lLevel = 0;
			if ( pEnvVar != 0 && coast::system::StrToL(lLevel, pEnvVar) ) {
				statisticLevel = lLevel;
			}
			InitFinisManager::IFMTrace("storage::Initialized\n");
		}
		~StorageInitializer();
		long GetStatisticLevel() {
			return statisticLevel;
		}
		//exchange this object when MT_Storage is used
		coast::storage::StorageHooksPtr fgTopHook;
	};
    typedef coast::utility::singleton_default<StorageInitializer> StorageInitializerSingleton;
}

namespace coast
{
	namespace storage
	{
		namespace
		{
			//flag to force global store temporarily
			bool forceGlobal = false;
			// the global allocator
			Allocator *globalPool = 0;
		}

		void Initialize() {
			if (StorageInitializerSingleton::instance().fgTopHook.get() && !forceGlobal) {
				StorageInitializerSingleton::instance().fgTopHook->Initialize();
			}
		}

		Allocator *DoGlobal() {
			if (!globalPool) {
				Initialize();
				globalPool = new GlobalAllocator();
			}
			return globalPool;
		}

		Allocator *Current() {
			if (StorageInitializerSingleton::instance().fgTopHook.get() && !forceGlobal) {
				return StorageInitializerSingleton::instance().fgTopHook->Current();
			}
			return DoGlobal();
		}

		Allocator *Global() {
			if (StorageInitializerSingleton::instance().fgTopHook.get() && !forceGlobal) {
				return StorageInitializerSingleton::instance().fgTopHook->Global();
			}
			return DoGlobal();
		}

		void DoFinalize() {
			// terminate global allocator and force printing statistics above level 1
			if (GetStatisticLevel() >= 1) {
				DoGlobal()->PrintStatistic(2);
			}
			if (globalPool) {
				delete globalPool;
				globalPool = 0;
			}
		}

		void Finalize() {
			if (StorageInitializerSingleton::instance().fgTopHook.get() && !forceGlobal) {
				StorageInitializerSingleton::instance().fgTopHook->Finalize();
			}
			DoFinalize();
		}
		long GetStatisticLevel() {
			return StorageInitializerSingleton::instance().GetStatisticLevel();
		}

		void PrintStatistic(long lLevel) {
			DoGlobal()->PrintStatistic(lLevel);
		}

		void registerHook(StorageHooksPtr h) {
			if (!h) {
				return;
			}
			h->SetOldHook(StorageInitializerSingleton::instance().fgTopHook);
			h->Initialize();
			StorageInitializerSingleton::instance().fgTopHook = h;
		}

		StorageHooksPtr unregisterHook() {
			StorageHooksPtr pOldHook = StorageInitializerSingleton::instance().fgTopHook;
			if (pOldHook.get()) {
				StorageInitializerSingleton::instance().fgTopHook = pOldHook->GetOldHook();
				pOldHook->Finalize();
			}
			return pOldHook;
		}

		ForceGlobalStorageEntry::ForceGlobalStorageEntry() {
			forceGlobal = true;
		}
		ForceGlobalStorageEntry::~ForceGlobalStorageEntry() {
			forceGlobal = false;
		}

		MemTracker *DoMakeMemTracker(const char *name) {
			return new MemTracker(name);
		}

		MemTracker *MakeMemTracker(const char *name, bool bThreadSafe) {
			if (StorageInitializerSingleton::instance().fgTopHook.get() && !forceGlobal) {
				return StorageInitializerSingleton::instance().fgTopHook->MakeMemTracker(name, bThreadSafe);
			}
			return DoMakeMemTracker(name);
		}
	} // namespace storage

	namespace memory
	{
		Allocator*& allocatorFor(void* ptr) throw () {
			return (reinterpret_cast<Allocator **>(ptr))[0L];
		}

		void *realPtrFor(void *ptr) throw () {
			return reinterpret_cast<char *>(ptr) - AlignedSize<Allocator *>::value;
		}

		void safeFree(Allocator *a, void *ptr) throw () {
			if (ptr) {
				a->Free(ptr); //lint !e1550
			}
		}
	} // namespace memory
} // namespace coast

namespace {
	StorageInitializer::~StorageInitializer() {
		coast::storage::Finalize();
		InitFinisManager::IFMTrace("storage::Finalized\n");
	}
}

Allocator::Allocator(long allocatorid) :
		fAllocatorId(allocatorid), fRefCnt(0), fTracker(new NullMemTracker("AllocatorNullTracker")) {
}

Allocator::~Allocator() {
	Assert(0 == fRefCnt);
} //lint !e1540

void *Allocator::Calloc(int n, size_t size) {
	void *ret = Alloc(AllocSize(n, size));
	if (ret && n * size > 0) {
		memset(ret, 0, n * size);
	}
	return ret;
}

void *Allocator::Malloc(size_t size) {
	return Alloc(AllocSize(size, 1));
}

void Allocator::Refresh() {
	// give the allocator opportunity to reorganize
}

size_t Allocator::AllocSize(size_t n, size_t size) {
	return (n * size) + coast::memory::AlignedSize<MemoryHeader>::value;
}

void *Allocator::ExtMemStart(void *vp) {
	if (vp) {
		Assert(((MemoryHeader *)vp)->fMagic == MemoryHeader::gcMagic);
		void *s = (((char *) (vp)) + coast::memory::AlignedSize<MemoryHeader>::value); //fUsableSize does *not* include header //lint !e429
		// superfluous, Calloc takes care: memset(s, '\0', mh->fUsableSize);
		return s;
	}
	return 0;
}

MemoryHeader *Allocator::RealMemStart(void *vp) {
	if (vp) {
		MemoryHeader *mh = (MemoryHeader *) (((char *) (vp)) - coast::memory::AlignedSize<MemoryHeader>::value);
		if (mh->fMagic == MemoryHeader::gcMagic) {
			return mh;
		}
	}
	return 0;
}

GlobalAllocator::GlobalAllocator() :
		Allocator(11223344L) {
	if (coast::storage::GetStatisticLevel() >= 1) {
		fTracker = MemTrackerPtr(coast::storage::MakeMemTracker("GlobalAllocator", false));
		fTracker->SetId(fAllocatorId);
	}
}

GlobalAllocator::~GlobalAllocator() {
}

Allocator::MemTrackerPtr GlobalAllocator::ReplaceMemTracker(MemTrackerPtr t) {
	std::swap(fTracker, t);
	return t;
}

ul_long GlobalAllocator::CurrentlyAllocated() {
	return fTracker->CurrentlyAllocated();
}

void GlobalAllocator::PrintStatistic(long lLevel) {
	fTracker->PrintStatistic(lLevel);
}

void *GlobalAllocator::Alloc(size_t allocSize) {
	void *vp = ::malloc(allocSize);

	if (vp) {
		MemoryHeader *mh = new (vp) MemoryHeader(allocSize - coast::memory::AlignedSize<MemoryHeader>::value, MemoryHeader::eUsedNotPooled);
		fTracker->TrackAlloc(mh);
		return ExtMemStart(mh); //lint !e593//lint !e429
	} else {
		const int bufSize = 256;
		static char crashmsg[bufSize] = { 0 };
		coast::system::SnPrintf(crashmsg, bufSize, "FATAL: GlobalAllocator::Alloc malloc of sz:%zu failed. I will crash :-(\n", allocSize);
		SystemLog::WriteToStderr(crashmsg, -1);
	}

	return 0;
}

void GlobalAllocator::Free(void* p, size_t sz) {
	Free(p);
	(void) sz;
}

void GlobalAllocator::Free(void *vp) {
	if (vp) {
		MemoryHeader *header = RealMemStart(vp);
		if (header && header->fMagic == MemoryHeader::gcMagic) {
			Assert(header->fMagic == MemoryHeader::gcMagic); // should combine magic with state
			Assert(header->fState & MemoryHeader::eUsed);
			fTracker->TrackFree(header);
			vp = header;
		}
		::free(vp);
	}
}

FoundationStorageHooks::FoundationStorageHooks() :
		fAllocator(new GlobalAllocator()) {
}
FoundationStorageHooks::~FoundationStorageHooks() {
}
void FoundationStorageHooks::DoInitialize() {
}
void FoundationStorageHooks::DoFinalize() {
}

char *itostorage::BoostPoolUserAllocatorGlobal::malloc(const size_type bytes) {
	return reinterpret_cast<char *>(coast::storage::Global()->Malloc(bytes));
}

void itostorage::BoostPoolUserAllocatorGlobal::free(char * const block) {
	coast::storage::Global()->Free(block);
}

char *itostorage::BoostPoolUserAllocatorCurrent::malloc(const size_type bytes) {
	return reinterpret_cast<char *>(coast::storage::Current()->Malloc(bytes));
}

void itostorage::BoostPoolUserAllocatorCurrent::free(char * const block) {
	coast::storage::Current()->Free(block);
}
