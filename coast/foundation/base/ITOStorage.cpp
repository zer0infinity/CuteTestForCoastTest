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
#include <stdio.h> //lint !e537//lint !e451
#include "InitFinisManager.h"
#if defined(WIN32)
#define snprintf	_snprintf
#endif


MemChecker::MemChecker(const char *scope, Allocator *a) :
		fAllocator(a), fSizeAllocated((a ? fAllocator->CurrentlyAllocated() : 0LL)), fScope(scope) {
}

MemChecker::~MemChecker() {
	TraceDelta("MemChecker.~MemChecker: ");
} //lint !e1579

void MemChecker::TraceDelta(const char *message) {
	l_long delta = CheckDelta();
	if (delta != 0) {
		char msgbuf[1024] = { '\0' };
		if (message) {
			SystemLog::WriteToStderr(message, strlen(message));
		}
		int bufsz = snprintf(msgbuf, sizeof(msgbuf), "\nMem Usage change by %.0f bytes in %s\nAllocator [%02ld]\n", (double) delta, fScope,
				(fAllocator ? fAllocator->GetId() : 0L));
		SystemLog::WriteToStderr(msgbuf, bufsz);
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
	if ( Coast::Storage::GetStatisticLevel() >= 3 ) {
		fpUsedList = new UsedListType();
	}
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

MemTracker::~MemTracker()
{
	if ( fpUsedList ) {
		delete fpUsedList;//lint !e1551
	}
	delete fpName;//lint !e1551//lint !e605
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
		SystemLog::Error(String(Coast::Storage::Global()).Append("memory blocks still in use for ").Append(fpName).Append(':'));
		UsedListType::const_iterator aUsedIterator;
		const size_t alignedSize = Coast::Memory::AlignedSize<MemoryHeader>::value;
		long lIdx = 0;
		for ( aUsedIterator = fpUsedList->begin(); aUsedIterator != fpUsedList->end(); ++lIdx, ++aUsedIterator) {
			MemoryHeader *pMH = *aUsedIterator;
			// reserve memory for the following text plus four times the buffer size for dumping as text
			String strOut(( 40L + ( ( pMH->fUsableSize + alignedSize ) * 4L ) ), Coast::Storage::Global());
			strOut.Append("Block ").Append(lIdx).Append('\n');
			strOut.Append("MemoryHeader:\n").Append(String((void *)pMH, alignedSize, Coast::Storage::Global()).DumpAsHex()).Append('\n');
			strOut.Append("Content:\n").Append(String((void *)((char *)pMH + alignedSize), pMH->fUsableSize, Coast::Storage::Global()).DumpAsHex()).Append('\n');
			SystemLog::WriteToStderr(strOut);
		}
	}
}

void MemTracker::PrintStatistic(long lLevel)
{
	if ( lLevel >= 2 ) {
		char buf[2048] = { 0 }; // safety margin for bytes
		snprintf(buf, sizeof(buf), "\nAllocator [%ld] [%s]\n"
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
		SystemLog::WriteToStderr(buf, strlen(buf));
	}
}

#include <boost/pool/detail/singleton.hpp>
namespace {
	class StatisticLevelInitializer {
		/* define the logging level of memory statistics by defining COAST_TRACE_STORAGE appropriately
						0: No pool statistic tracing, even not for excess memory nor GlobalAllocator usage
						1: Trace overall statistics
						2: + trace detailed statistics
						3: + keep track of allocated blocks to trace them in case they were not freed */
		long statisticLevel;
	public:
		StatisticLevelInitializer() {
			const char *pEnvVar = getenv("COAST_TRACE_STORAGE");
			long lLevel = ((pEnvVar != 0) ? atol(pEnvVar) : 0);
			statisticLevel = lLevel;
			InitFinisManager::IFMTrace("Storage::Initialized\n");
		}
		~StatisticLevelInitializer() {
			InitFinisManager::IFMTrace("Storage::Finalized\n");
		}
		long GetStatisticLevel() {
			return statisticLevel;
		}
	};
    typedef boost::details::pool::singleton_default<StatisticLevelInitializer> StatisticLevelInitializerSingleton;
}

namespace Coast
{
	namespace Storage
	{
		namespace
		{
			//exchange this object when MT_Storage is used
			StorageHooksPtr fgTopHook;

			//flag to force global store temporarily
			bool forceGlobal = false;

			// the global allocator
			Allocator *globalPool = 0;
		} // anonymous namespace

		void Initialize() {
			if (fgTopHook && !forceGlobal) {
				fgTopHook->Initialize();
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
			if (fgTopHook && !forceGlobal) {
				return fgTopHook->Current();
			}
			return DoGlobal();
		}

		Allocator *Global() {
			if (fgTopHook && !forceGlobal) {
				return fgTopHook->Global();
			}
			return DoGlobal();
		}

		void DoFinalize() {
			// terminate global allocator and force printing statistics above level 1
			if (GetStatisticLevel() >= 1) {
				DoGlobal()->PrintStatistic(2);
			}
		}

		void Finalize() {
			if (fgTopHook && !forceGlobal) {
				fgTopHook->Finalize();
			}
			DoFinalize();
		}
		long GetStatisticLevel() {
			return StatisticLevelInitializerSingleton::instance().GetStatisticLevel();
		}

		void PrintStatistic(long lLevel) {
			DoGlobal()->PrintStatistic(lLevel);
		}

		void registerHooks(StorageHooksPtr h) {
			if (!h) {
				return;
			}
			h->SetOldHook(fgTopHook);
			h->Initialize();
			fgTopHook = h;
		}

		StorageHooksPtr unregisterHooks() {
			StorageHooksPtr pOldHook = fgTopHook;
			fgTopHook = fgTopHook->GetOldHook();
			if (pOldHook) {
				pOldHook->Finalize();
			}
			return pOldHook;
		}

		void ForceGlobalStorage(bool b) {
			forceGlobal = b;
		}

		MemTracker *DoMakeMemTracker(const char *name) {
			return new MemTracker(name);
		}

		MemTracker *MakeMemTracker(const char *name, bool bThreadSafe) {
			if (fgTopHook && !forceGlobal) {
				return fgTopHook->MakeMemTracker(name, bThreadSafe);
			}
			return DoMakeMemTracker(name);
		}
	} // namespace Storage

	namespace Memory
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
	} // namespace Memory
} // namespace Coast

Allocator::Allocator(long allocatorid) :
		fAllocatorId(allocatorid), fRefCnt(0), fTracker(new NullMemTracker()) {
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
	return (n * size) + Coast::Memory::AlignedSize<MemoryHeader>::value;
}

void *Allocator::ExtMemStart(void *vp) {
	if (vp) {
		Assert(((MemoryHeader *)vp)->fMagic == MemoryHeader::gcMagic);
		void *s = (((char *) (vp)) + Coast::Memory::AlignedSize<MemoryHeader>::value); //fUsableSize does *not* include header //lint !e429
		// superfluous, Calloc takes care: memset(s, '\0', mh->fUsableSize);
		return s;
	}
	return 0;
}

MemoryHeader *Allocator::RealMemStart(void *vp) {
	if (vp) {
		MemoryHeader *mh = (MemoryHeader *) (((char *) (vp)) - Coast::Memory::AlignedSize<MemoryHeader>::value);
		if (mh->fMagic == MemoryHeader::gcMagic) {
			return mh;
		}
	}
	return 0;
}

GlobalAllocator::GlobalAllocator() :
		Allocator(11223344L) {
	if (Coast::Storage::GetStatisticLevel() >= 1) {
		fTracker = MemTrackerPtr(Coast::Storage::MakeMemTracker("GlobalAllocator", false));
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
		MemoryHeader *mh = new (vp) MemoryHeader(allocSize - Coast::Memory::AlignedSize<MemoryHeader>::value, MemoryHeader::eUsedNotPooled);
		fTracker->TrackAlloc(mh);
		return ExtMemStart(mh); //lint !e593//lint !e429
	} else {
		static char crashmsg[255] = { 0 };
		snprintf(crashmsg, 254, "FATAL: GlobalAllocator::Alloc malloc of sz:%zu failed. I will crash :-(\n", allocSize);
		SystemLog::WriteToStderr(crashmsg, strlen(crashmsg));
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

char *ITOStorage::BoostPoolUserAllocatorGlobal::malloc(const size_type bytes) {
	return reinterpret_cast<char *>(Coast::Storage::Global()->Malloc(bytes));
}

void ITOStorage::BoostPoolUserAllocatorGlobal::free(char * const block) {
	Coast::Storage::Global()->Free(block);
}

char *ITOStorage::BoostPoolUserAllocatorCurrent::malloc(const size_type bytes) {
	return reinterpret_cast<char *>(Coast::Storage::Current()->Malloc(bytes));
}

void ITOStorage::BoostPoolUserAllocatorCurrent::free(char * const block) {
	Coast::Storage::Current()->Free(block);
}
