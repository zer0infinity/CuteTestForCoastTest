/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MT_Storage.h"

//--- project modules used -----------------------------------------------------
#include "InitFinisManagerMTFoundation.h"

//--- standard modules used ----------------------------------------------------
#include "Threads.h"
#include "PoolAllocator.h"
#include "ITOString.h"
#include "SysLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

#if 1
#define TrackLockerInit(lockvar) , lockvar(0)
#define TrackLockerDef(lockvar)	volatile long lockvar

class EXPORTDECL_MTFOUNDATION CurrLockerEntry
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
			char buf[256] = { 0 };
			snprintf(buf, sizeof(buf), "\n%s:%ld Another Locker entered already! otherThreadId:%ld currentThreadId:%ld\n", pFile, lLine, lOther, Thread::MyId());
			SysLog::WriteToStderr(buf, strlen(buf));
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
	//! \param poolid use poolid to distinguish more than one pool
	//! \param poolSize size of pre-allocated pool in kBytes, default 1MByte
	MTPoolAllocator(long poolid, u_long poolSize = 1024, u_long maxKindOfBucket = 10)
		: PoolAllocator(poolid, poolSize, maxKindOfBucket) TrackLockerInit(fCurrLockerId) {}
	//! destroy a pool only if its empty, i.e. all allocated bytes are freed
	virtual ~MTPoolAllocator() { }
	//! implement hook for freeing memory
	virtual inline void Free(void *vp) {
		TrackLocker(fCurrLockerId);
		PoolAllocator::Free(vp);
	}

protected:
	TrackLockerDef(fCurrLockerId);
	//!implement hook for allocating memory using bucketing
	virtual inline void *Alloc(u_long allocSize) {
		TrackLocker(fCurrLockerId);
		return PoolAllocator::Alloc(allocSize);
	}
};

//------------- Utilities for Memory Tracking (multi threaded) --------------

MT_MemTracker::MT_MemTracker(const char *name, long lId)
	: MemTracker(name)
{
	if ( !CREATEMUTEX(fMutex) ) {
		SysLog::Error("Mutex create failed");
	}
	SetId( lId );
}

MT_MemTracker::~MT_MemTracker()
{
	if ( !DELETEMUTEX(fMutex) ) {
		SysLog::Error("Mutex delete failed");
	}
}

void *MT_MemTracker::operator new(size_t size)
{
	// never allocate on allocator, because we are tracking exactly the one...
	void *vp = ::calloc(1, sizeof(MT_MemTracker));
	return vp;
}

void *MT_MemTracker::operator new(size_t size, Allocator *)
{
	// never allocate on allocator, because we are tracking exactly the one...
	void *vp = ::calloc(1, sizeof(MT_MemTracker));
	return vp;
}

void MT_MemTracker::operator delete(void *vp)
{
	if (vp) {
		::operator delete(vp);
	}
}

void MT_MemTracker::TrackAlloc(u_long allocSz)
{
	if ( !LOCKMUTEX(fMutex) ) {
		SysLog::Error("Mutex lock failed");
	}
	MemTracker::TrackAlloc(allocSz);
	if ( !UNLOCKMUTEX(fMutex) ) {
		SysLog::Error("Mutex unlock failed");
	}
}

void MT_MemTracker::TrackFree(u_long allocSz)
{
	if ( !LOCKMUTEX(fMutex) ) {
		SysLog::Error("Mutex lock failed");
	}
	MemTracker::TrackFree(allocSz);
	if ( !UNLOCKMUTEX(fMutex) ) {
		SysLog::Error("Mutex unlock failed");
	}
}

l_long MT_MemTracker::CurrentlyAllocated()
{
	l_long l;
	if ( !LOCKMUTEX(fMutex) ) {
		SysLog::Error("Mutex lock failed");
	}
	l = fAllocated;
	if ( !UNLOCKMUTEX(fMutex) ) {
		SysLog::Error("Mutex lock failed");
	}
	return  l;
}

void MT_MemTracker::PrintStatistic(long lLevel)
{
	if ( !LOCKMUTEX(fMutex) ) {
		SysLog::Error("Mutex lock failed");
	}
	MemTracker::PrintStatistic(lLevel);
	if ( !UNLOCKMUTEX(fMutex) ) {
		SysLog::Error("Mutex lock failed");
	}
}

class EXPORTDECL_MTFOUNDATION MTStorageHooks : public StorageHooks
{
public:
	MTStorageHooks();
	virtual ~MTStorageHooks();

	virtual void Initialize();
	virtual void Finalize();
	virtual Allocator *Global();

	virtual Allocator *Current();

	/*! allocate a memory tracker object
		\param name name of the tracker
		\param bThreadSafe specify if tracker must be thread safe or not - not used from within foundation
		\return poniter to a newly created MemTracker object */
	virtual MemTracker *MakeMemTracker(const char *name, bool bThreadSafe);

private:
	bool fgInitialized;
};

MemTracker *MT_Storage::fOldTracker = NULL;

static MTStorageHooks *sgpMTHooks = NULL;

//---- MT_Storage ------------------------------------------
THREADKEY MT_Storage::fgAllocatorKey = 0;
SimpleMutex *MT_Storage::fgpAllocatorInit = NULL;
bool MT_Storage::fgInitialized = false;

struct AllocList {
	Allocator *wdallocator;
	AllocList *next;
};

static AllocList *fgPoolAllocatorList = 0;

class EXPORTDECL_MTFOUNDATION MTStorageInitializer : public InitFinisManagerMTFoundation
{
public:
	MTStorageInitializer(unsigned int uiPriority)
		: InitFinisManagerMTFoundation(uiPriority) {
		IFMTrace("MTStorageInitializer created\n");
	}

	~MTStorageInitializer()
	{}

	virtual void DoInit() {
		IFMTrace("MTStorageInitializer::DoInit\n");
		if (THRKEYCREATE(MT_Storage::fgAllocatorKey, 0)) {
			SysLog::Error("TlsAlloc of MT_Storage::fgAllocatorKey failed");
		}
		MT_Storage::fgpAllocatorInit = new SimpleMutex("AllocatorInit");
		MT_Storage::Initialize();
	}

	virtual void DoFinis() {
		IFMTrace("MTStorageInitializer::DoFinis\n");
		MT_Storage::Finalize();
		delete MT_Storage::fgpAllocatorInit;
		MT_Storage::fgpAllocatorInit = NULL;
		if (THRKEYDELETE(MT_Storage::fgAllocatorKey) != 0) {
			SysLog::Error("TlsFree of MT_Storage::fgAllocatorKey failed" );
		}
	}
};

static MTStorageInitializer *psgMTStorageInitializer = new MTStorageInitializer(10);

void MT_Storage::Initialize()
{
	StatTrace(MT_Storage.Initialize, "entering", Storage::Global());
	// must be called before threading is activated
	if ( !fgInitialized ) {
		sgpMTHooks = new MTStorageHooks();
		Storage::SetHooks(sgpMTHooks);
		// switch to thread safe memory tracker
		Allocator *a = Storage::Global();
		if ( a ) {
			fOldTracker = a->ReplaceMemTracker(Storage::MakeMemTracker("MTGlobalAllocator", true));
		}
		fgInitialized = true;
	}
	StatTrace(MT_Storage.Initialize, "leaving", Storage::Global());
}

void MT_Storage::Finalize()
{
	StatTrace(MT_Storage.Finalize, "entering", Storage::Global());
	if ( fgInitialized ) {
		// terminate pool allocators
		{
			SimpleMutexEntry me(*fgpAllocatorInit);
			me.Use();
			while (fgPoolAllocatorList) {
				AllocList *elmt = fgPoolAllocatorList;
				if ( elmt->wdallocator->GetId() != Storage::DoGlobal()->GetId() ) {
					SYSERROR("unfreed allocator found id: " << elmt->wdallocator->GetId());
					delete elmt->wdallocator;
				}
				fgPoolAllocatorList = elmt->next;
				::free(elmt);
			}
		}
		Allocator *a = Storage::Global();
		if ( a ) {
			a->PrintStatistic();
			if ( fOldTracker ) {
				delete a->ReplaceMemTracker(fOldTracker);
				fOldTracker = NULL;
			}

		}
		StorageHooks *pOldHook = Storage::SetHooks(NULL);
		if ( pOldHook == sgpMTHooks ) {
			delete sgpMTHooks;
			sgpMTHooks = NULL;
		}
		fgInitialized = false;
	}
	StatTrace(MT_Storage.Finalize, "leaving", Storage::Global());
}

void MT_Storage::RefAllocator(Allocator *wdallocator)
{
	StartTrace1(MT_Storage.RefAllocator, "Id:" << (wdallocator ? wdallocator->GetId() : -1L));
	if ( fgInitialized ) {
		if ( wdallocator ) {
			// in mt case we need to control ref counting with mutexes
			SimpleMutexEntry me(*fgpAllocatorInit);
			me.Use();

			AllocList *elmt = (AllocList *)::calloc(1, sizeof(AllocList));
			// catch memory allocation problem
			if (!elmt) {
				static const char crashmsg[] = "FATAL: MT_Storage::RefAllocator calloc failed. I will crash :-(\n";
				SysLog::WriteToStderr(crashmsg, sizeof(crashmsg));
				SysLog::Error("allocation failed for RefAllocator");
				return;
			}

			// normal case everything ok
			wdallocator->Ref();
			Trace("refcount is now:" << wdallocator->RefCnt());
			// update linked list
			elmt->wdallocator = wdallocator;
			elmt->next = fgPoolAllocatorList;
			fgPoolAllocatorList = elmt;
		}
	} else {
		SYSERROR("MT_Storage not initialized!");
	}
}

void MT_Storage::UnrefAllocator(Allocator *wdallocator)
{
	StatTrace(MT_Storage.UnrefAllocator, "Id:" << (wdallocator ? wdallocator->GetId() : -1L) << " --- entering ---", Storage::Global());
	if ( fgInitialized ) {
		if (wdallocator) {	// just to be robust wdallocator == 0 should not happen
			SimpleMutexEntry me(*fgpAllocatorInit);
			me.Use();
			wdallocator->Unref();
			StatTrace(MT_Storage.UnrefAllocator, "refcount is now:" << wdallocator->RefCnt(), Storage::Global());

			if ( wdallocator->RefCnt() <= 0 ) {
				// remove pool allocator
				AllocList *elmt = fgPoolAllocatorList;
				AllocList *prev = fgPoolAllocatorList;

				while (elmt && (elmt->wdallocator != wdallocator)) {
					prev = elmt;
					elmt = elmt->next;
				}
				if ( elmt ) {
					if ( elmt == fgPoolAllocatorList ) {
						fgPoolAllocatorList = elmt->next;
					} else {
						prev->next = elmt->next;
					}

					delete elmt->wdallocator;
					::free(elmt);
				}
			}
		}
	} else {
		SYSERROR("MT_Storage not initialized!");
	}
	StatTrace(MT_Storage.UnrefAllocator, "Id:" << (wdallocator ? wdallocator->GetId() : -1L) << " --- leaving ---", Storage::Global());
}

bool MT_Storage::RegisterThread(Allocator *wdallocator)
{
	StartTrace(MT_Storage.RegisterThread);
	if ( fgInitialized ) {
		// can only be used once for the same thread
		Allocator *oldAllocator = 0;

		// check for old allocator, and if any, dont override it
		GETTLSDATA(MT_Storage::fgAllocatorKey, oldAllocator, Allocator);
		if ( oldAllocator == NULL ) {
			return !SETTLSDATA(MT_Storage::fgAllocatorKey, wdallocator);
		}
	} else {
		SYSERROR("MT_Storage not initialized!");
	}
	return false;
}

bool MT_Storage::UnregisterThread()
{
	StartTrace(MT_Storage.UnregisterThread);
	if ( fgInitialized ) {
		// check for current allocator
		Allocator *oldAllocator = 0;

		// check for old allocator, and if any delete it
		GETTLSDATA(MT_Storage::fgAllocatorKey, oldAllocator, Allocator);
		if ( oldAllocator ) {
			return !SETTLSDATA(MT_Storage::fgAllocatorKey, 0);
		}
	} else {
		SYSERROR("MT_Storage not initialized!");
	}
	return false;
}

Allocator *MT_Storage::MakePoolAllocator(u_long poolStorageSize, u_long numOfPoolBucketSizes, long lPoolId)
{
	StartTrace(MT_Storage.MakePoolAllocator);
	Allocator *newPoolAllocator = new MTPoolAllocator(lPoolId, poolStorageSize, numOfPoolBucketSizes);
	if (!newPoolAllocator) {
		String msg("allocation of PoolStorage: ");
		msg << (long)poolStorageSize << ", " << (long)numOfPoolBucketSizes << " failed";
		SysLog::Error(msg);
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

MTStorageHooks::MTStorageHooks()
	: fgInitialized(false)
{
}

MTStorageHooks::~MTStorageHooks()
{
}

Allocator *MTStorageHooks::Global()
{
	return Storage::DoGlobal();
}

MemTracker *MTStorageHooks::MakeMemTracker(const char *name, bool bThreadSafe)
{
	MemTracker *pTracker = NULL;
	if ( bThreadSafe ) {
		pTracker = new MT_MemTracker(name, 55667788);
	} else {
		pTracker = Storage::DoMakeMemTracker(name);
	}
	return pTracker;
}

Allocator *MTStorageHooks::Current()
{
	if ( fgInitialized ) {
		Allocator *wdallocator = 0;
		// determine which allocator to use
		if ( GETTLSDATA(MT_Storage::fgAllocatorKey, wdallocator, Allocator) && wdallocator ) {
			return wdallocator;
		}
	}
	return Global();
}

void MTStorageHooks::Initialize()
{
	if ( !fgInitialized ) {
		fgInitialized = true;
	}
}

void MTStorageHooks::Finalize()
{
	if ( fgInitialized ) {
		fgInitialized = false;
	}
}
