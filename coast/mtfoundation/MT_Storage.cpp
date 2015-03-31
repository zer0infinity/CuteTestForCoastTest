/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MT_Storage.h"
#include "InitFinisManager.h"
#include "Threads.h"
#include "PoolAllocator.h"
#include "SystemLog.h"
#include "SystemBase.h"
#include <cstring>

#if 1
#define TrackLockerInit(lockvar) , lockvar(0)
#define TrackLockerDef(lockvar)	volatile long lockvar

class CurrLockerEntry
{
	volatile long &frLockerId;
public:
	CurrLockerEntry(volatile long &rLockerId, const char *pFile, long lLine)
		: frLockerId(rLockerId) {
		CheckException(pFile, lLine);
		frLockerId = Thread::MyId();
	}
	~CurrLockerEntry() {
		frLockerId = 0;
	}
	inline void CheckException(const char *pFile, long lLine) {
		if ( frLockerId != 0 ) {
			long lOther = frLockerId;
			const int bufSize = 256;
			char buf[bufSize] = { 0 };
			coast::system::SnPrintf(buf, bufSize, "\n%s:%ld Another Locker entered already! otherThreadId:%ld currentThreadId:%ld\n", pFile, lLine, lOther, Thread::MyId());
			SystemLog::WriteToStderr(buf, -1);
		}
	}
};

#define TrackLocker(lockvar)	CurrLockerEntry aEntry(lockvar, __FILE__, __LINE__)

#else

#define TrackLockerInit(lockvar)
#define TrackLockerDef(lockvar)
#define TrackLocker(lockvar)

#endif

class MTPoolAllocator: public PoolAllocator
{
public:
	//! create and initialize a pool allocator
	/*! \param poolid use poolid to distinguish more than one pool
		\param poolSize size of pre-allocated pool in kBytes, default 1MByte
		\param maxKindOfBucket number of different allocation units within PoolAllocator, starts at 16 bytes and doubles the size for maxKindOfBucket times. So maxKindOfBucket=10 will give a max usable size of 8192 bytes. */
	MTPoolAllocator(long poolid, u_long poolSize = 1024, u_long maxKindOfBucket = 10)
		: PoolAllocator(poolid, poolSize, maxKindOfBucket) TrackLockerInit(fCurrLockerId) {}
	//! destroy a pool only if its empty, i.e. all allocated bytes are freed
	virtual ~MTPoolAllocator() { }
	virtual inline void Free(void *vp, size_t sz) {
		PoolAllocator::Free(vp, sz);
	}
	//! implement hook for freeing memory
	virtual inline void Free(void *vp) {
		TrackLocker(fCurrLockerId);
		PoolAllocator::Free(vp);
	}

protected:
	TrackLockerDef(fCurrLockerId);
	//!implement hook for allocating memory using bucketing
	virtual inline void *Alloc(size_t allocSize) {
		TrackLocker(fCurrLockerId);
		return PoolAllocator::Alloc(allocSize);
	}
};

MT_MemTracker::MT_MemTracker(const char *name, long lId)
	: MemTracker(name)
{
	int iRet = 0;
	if ( !CREATEMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex create failed: " << SystemLog::SysErrorMsg(iRet));
	}
	SetId( lId );
}

MT_MemTracker::~MT_MemTracker()
{
	int iRet = 0;
	if ( !DELETEMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex delete failed: " << SystemLog::SysErrorMsg(iRet));
	}
}


void MT_MemTracker::TrackAlloc(MemoryHeader *mh)
{
	int iRet = 0;
	if ( !LOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
	MemTracker::TrackAlloc(mh);
	if ( !UNLOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex unlock failed" << SystemLog::SysErrorMsg(iRet));
	}
}

void MT_MemTracker::TrackFree(MemoryHeader *mh)
{
	int iRet = 0;
	if ( !LOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
	MemTracker::TrackFree(mh);
	if ( !UNLOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex unlock failed: " << SystemLog::SysErrorMsg(iRet));
	}
}

ul_long MT_MemTracker::CurrentlyAllocated()
{
	ul_long l;
	int iRet = 0;
	if ( !LOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
	l = fAllocated;
	if ( !UNLOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
	return  l;
}

void MT_MemTracker::PrintStatistic(long lLevel)
{
	int iRet = 0;
	if ( !LOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
	MemTracker::PrintStatistic(lLevel);
	if ( !UNLOCKMUTEX(fMutex, iRet) ) {
		SYSERROR("Mutex lock failed: " << SystemLog::SysErrorMsg(iRet));
	}
}

class MTStorageHooks: public StorageHooks {
	bool fgInitialized;
public:
	MTStorageHooks() :
			fgInitialized(false) {
	}
protected:
	virtual void DoInitialize() {
		if (!fgInitialized) {
			fgInitialized = true;
		}
	}

	virtual void DoFinalize() {
		if (fgInitialized) {
			fgInitialized = false;
		}
	}
	virtual Allocator *DoGlobal() {
		return coast::storage::DoGlobal();
	}
	virtual Allocator *DoCurrent();

	/*! allocate a memory tracker object
	 \param name name of the tracker
	 \param bThreadSafe specify if tracker must be thread safe or not - not used from within foundation
	 \return poniter to a newly created MemTracker object */
	virtual MemTracker *DoMakeMemTracker(const char *name, bool bThreadSafe) {
		MemTracker *pTracker = NULL;
		if (bThreadSafe) {
			pTracker = new MT_MemTracker(name, 55667788);
		} else {
			pTracker = coast::storage::DoMakeMemTracker(name);
		}
		return pTracker;
	}
};

#include "singleton.hpp"
#include <boost/shared_ptr.hpp>

namespace {
	struct AllocList {
		Allocator *wdallocator;
		AllocList *next;
	};
	class MTStorageInitializer {
		typedef boost::shared_ptr<SimpleMutex> SimpleMutexPtr;
		typedef coast::storage::StorageHooksPtr StorageHooksPtr;
		SimpleMutexPtr fAllocatorInit;
		THREADKEY fAllocatorKey;
		Allocator::MemTrackerPtr fOldTracker;
		StorageHooksPtr fMTHooks;
		AllocList *fPoolAllocatorList;
		void Initialize() {
			StatTrace(MT_Storage.Initialize, "entering", coast::storage::Global());
			// must be called before threading is activated
			fMTHooks = StorageHooksPtr(new MTStorageHooks());
			coast::storage::registerHook(fMTHooks);
			// switch to thread safe memory tracker if enabled through COAST_TRACE_STORAGE
			Allocator *a = coast::storage::Global();
			if (a && coast::storage::GetStatisticLevel() >= 1) {
				fOldTracker = a->ReplaceMemTracker(Allocator::MemTrackerPtr(coast::storage::MakeMemTracker("MTGlobalAllocator", true)));
			}
			StatTrace(MT_Storage.Initialize, "leaving", coast::storage::Global());
		}
		void Finalize()
		{
			StatTrace(MT_Storage.Finalize, "entering", coast::storage::Global());
			// terminate pool allocators
			{
				LockUnlockEntry me(*fAllocatorInit);
				while (fPoolAllocatorList) {
					AllocList *elmt = fPoolAllocatorList;
					if ( elmt->wdallocator->GetId() != coast::storage::DoGlobal()->GetId() ) {
						SYSERROR("unfreed allocator found id: " << elmt->wdallocator->GetId());
						delete elmt->wdallocator;
					}
					fPoolAllocatorList = elmt->next;
					::free(elmt);
				}
			}
			Allocator *a = coast::storage::Global();
			if ( a ) {
				if ( fOldTracker ) {
					Allocator::MemTrackerPtr tmpTracker = a->ReplaceMemTracker(fOldTracker);
					StatTrace(MT_Storage.Finalize, "setting MemTracker back from [" << ( tmpTracker ? tmpTracker->GetName() : "NULL" ) << "] to [" << fOldTracker->GetName() << "]", coast::storage::Global());
					if ( tmpTracker && coast::storage::GetStatisticLevel() >= 1 ) {
						tmpTracker->PrintStatistic(2);
					}
					fOldTracker.reset();
				}
			}
			coast::storage::StorageHooksPtr pOldHook = coast::storage::unregisterHook();
			if ( pOldHook != fMTHooks ) {
				SYSERROR("unregistered different hooks, oops");
			}
			StatTrace(MT_Storage.Finalize, "leaving", coast::storage::Global());
		}
	public:
		MTStorageInitializer() : fAllocatorInit(new SimpleMutex("AllocatorInit", coast::storage::Global())), fAllocatorKey(0), fOldTracker(), fMTHooks(), fPoolAllocatorList(0) {
			if (THRKEYCREATE(fAllocatorKey, 0)) {
				SystemLog::Error("TlsAlloc of fAllocatorKey failed");
			}
			Initialize();
			InitFinisManager::IFMTrace("MTstorage::Initialized\n");
		}
		~MTStorageInitializer() {
			Finalize();
			if (THRKEYDELETE(fAllocatorKey) != 0) {
				SystemLog::Error("TlsFree of fAllocatorKey failed" );
			}
			InitFinisManager::IFMTrace("MTstorage::Finalized\n");
		}
		void RefAllocator(Allocator *wdallocator) {
			StartTrace1(MT_Storage.RefAllocator, "Id:" << (wdallocator ? wdallocator->GetId() : -1L));
			if ( wdallocator ) {
				// in mt case we need to control ref counting with mutexes
				LockUnlockEntry me(*fAllocatorInit);
				AllocList *elmt = (AllocList *)::calloc(1, sizeof(AllocList));
				// catch memory allocation problem
				if (!elmt) {
					static const char crashmsg[] = "FATAL: MT_Storage::RefAllocator calloc failed. I will crash :-(\n";
					SystemLog::WriteToStderr(crashmsg, sizeof(crashmsg));
					SystemLog::Error("allocation failed for RefAllocator");
					return;
				}
				wdallocator->Ref();
				Trace("refcount is now:" << wdallocator->RefCnt());
				// update linked list
				elmt->wdallocator = wdallocator;
				elmt->next = fPoolAllocatorList;
				fPoolAllocatorList = elmt;
			}
		}
		void UnrefAllocator(Allocator *wdallocator) {
			const long allocId = wdallocator ? wdallocator->GetId() : -1L;
			(void)allocId;  // to prevent unused compiler warning occurring when trace is disabled
			StatTrace(MT_Storage.UnrefAllocator, "Id:" << allocId << " --- entering ---", coast::storage::Global());
			if (wdallocator) {	// just to be robust wdallocator == 0 should not happen
				LockUnlockEntry me(*fAllocatorInit);
				wdallocator->Unref();
				StatTrace(MT_Storage.UnrefAllocator, "refcount is now:" << wdallocator->RefCnt(), coast::storage::Global());
				if ( wdallocator->RefCnt() <= 0 ) {
					AllocList *elmt = fPoolAllocatorList;
					AllocList *prev = fPoolAllocatorList;
					while (elmt && (elmt->wdallocator != wdallocator)) {
						prev = elmt;
						elmt = elmt->next;
					}
					if ( elmt ) {
						if ( elmt == fPoolAllocatorList ) {
							fPoolAllocatorList = elmt->next;
						} else {
							prev->next = elmt->next;
						}
						delete elmt->wdallocator;
						::free(elmt);
					}
				}
			}
			StatTrace(MT_Storage.UnrefAllocator, "Id:" << allocId << " --- leaving ---", coast::storage::Global());
		}
		bool RegisterThread(Allocator *wdallocator) {
			// can only be used once for the same thread
			Allocator *oldAllocator = 0;
			// check for old allocator, and if any, dont override it
			GETTLSDATA(getAllocatorKey(), oldAllocator, Allocator);
			if (oldAllocator == NULL) {
				StatTrace(MT_Storage.RegisterThread, "setting Allocator:" << (long)wdallocator << " to ThreadLocalStorage",
						coast::storage::Global());
				return !SETTLSDATA(getAllocatorKey(), wdallocator);
			}
			return false;
		}
		bool UnregisterThread() {
			Allocator *oldAllocator = 0;
			// check for old allocator, and if any delete it
			GETTLSDATA(getAllocatorKey(), oldAllocator, Allocator);
			if (oldAllocator) {
				StatTrace(MT_Storage.UnregisterThread, "removing Allocator:" << (long)oldAllocator << " of ThreadLocalStorage",
						coast::storage::Global());
				return !SETTLSDATA(getAllocatorKey(), 0);
			}
			return false;
		}
		THREADKEY getAllocatorKey() const {
			return fAllocatorKey;
		}
	};
    typedef coast::utility::singleton_default<MTStorageInitializer> MTStorageInitializerSingleton;
}

Allocator *MTStorageHooks::DoCurrent() {
	if (fgInitialized) {
		Allocator *wdallocator = 0;
		// determine which allocator to use
		if (GETTLSDATA(MTStorageInitializerSingleton::instance().getAllocatorKey(), wdallocator, Allocator) && wdallocator) {
			return wdallocator;
		}
	}
	return Global();
}

void MT_Storage::RefAllocator(Allocator *wdallocator) {
	MTStorageInitializerSingleton::instance().RefAllocator(wdallocator);
}
void MT_Storage::UnrefAllocator(Allocator *wdallocator) {
	MTStorageInitializerSingleton::instance().UnrefAllocator(wdallocator);
}
bool MT_Storage::RegisterThread(Allocator *wdallocator) {
	return MTStorageInitializerSingleton::instance().RegisterThread(wdallocator);
}
bool MT_Storage::UnregisterThread() {
	return MTStorageInitializerSingleton::instance().UnregisterThread();
}
THREADKEY MT_Storage::getAllocatorKey() {
	return MTStorageInitializerSingleton::instance().getAllocatorKey();
}

Allocator *MT_Storage::MakePoolAllocator(u_long poolStorageSize, u_long numOfPoolBucketSizes, long lPoolId)
{
	StartTrace(MT_Storage.MakePoolAllocator);
	Allocator *newPoolAllocator = new MTPoolAllocator(lPoolId, poolStorageSize, numOfPoolBucketSizes);
	if (!newPoolAllocator) {
		String msg("allocation of PoolStorage: ");
		msg << (long)poolStorageSize << ", " << (long)numOfPoolBucketSizes << " failed";
		SystemLog::Error(msg);
		return newPoolAllocator;
	}
	if (newPoolAllocator->RefCnt() <= -1) {
		// we managed to allocate a pool allocator
		// but the object failed to allocate the pool storage
		delete newPoolAllocator;
		newPoolAllocator = 0;
	}
	return newPoolAllocator;
}
