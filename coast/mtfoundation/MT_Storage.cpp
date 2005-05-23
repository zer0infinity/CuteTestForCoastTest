/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MT_Storage.h"

//--- standard modules used ----------------------------------------------------
#include "Threads.h"
#include "PoolAllocator.h"
#include "SysLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if !defined(WIN32)
#include <stdlib.h>
#endif

THREADKEY MT_Storage::fgAllocatorKey = 0;

#define FindAllocator(wdallocator)									\
		GETTLSDATA(MT_Storage::fgAllocatorKey, wdallocator, Allocator)

void TLSDestructor(void *)
{
	// destructor function for thread local storage
}

static Mutex *gsAllocatorInit = 0;		// protect all data structures used by MT_Storage

#if 0
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
			char buf[256];
			snprintf(buf, sizeof(buf), "\n%s:%d Another Locker entered already! otherThreadId:%d currentThreadId:%d\n", pFile, lLine, lOther, Thread::MyId());
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

#ifdef MEM_DEBUG
//------------- Utilities for Memory Tracking (multi threaded) --------------

MT_MemTracker::MT_MemTracker(const char *name)
	: MemTracker(name)
{
	if ( !CREATEMUTEX(fMutex) ) {
		SysLog::Error("Mutex create failed");
	}
}

MT_MemTracker::~MT_MemTracker()
{
	if ( !DELETEMUTEX(fMutex) ) {
		SysLog::Error("Mutex delete failed");
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

void MT_MemTracker::PrintStatistic()
{
	if ( !LOCKMUTEX(fMutex) ) {
		SysLog::Error("Mutex lock failed");
	}
	MemTracker::PrintStatistic();
	if ( !UNLOCKMUTEX(fMutex) ) {
		SysLog::Error("Mutex lock failed");
	}
}
#endif

class EXPORTDECL_MTFOUNDATION MTStorageHooks : public StorageHooks
{
public:
	virtual void Initialize();
	virtual void Finalize();
	virtual Allocator *Global();

	virtual Allocator *Current();
#ifdef MEM_DEBUG
	virtual MemTracker *MakeMemTracker(const char *name);
#endif
	static bool fgInitialized;
};

static MTStorageHooks sgMTHooks;

//---- MT_Storage ------------------------------------------
struct AllocList {
	Allocator *wdallocator;
	AllocList *next;
};

static AllocList *fgPoolAllocatorList = 0;
bool MTStorageHooks::fgInitialized = false; // flag for memorizing initialization

void MT_Storage::Initialize()
{
	StartTrace(MT_Storage.Initialize);
	// must be called before threading is activated
	static bool once = false;
	if (!once) {
		gsAllocatorInit	= new Mutex("AllocatorInit");
		Storage::SetHooks(&sgMTHooks);
		Storage::Initialize(); // re-init with MT hook

#ifdef MEM_DEBUG
		// switch to thread safe memory tracker
		Allocator *a = Storage::Global();
		if (a) {
			a->ReplaceMemTracker(Storage::MakeMemTracker("MTGlobalAllocator"));
		}
#endif
		once = true;
	}
}

void MT_Storage::RefAllocator(Allocator *wdallocator)
{
	StartTrace1(MT_Storage.RefAllocator, "Id:" << (wdallocator ? wdallocator->GetId() : -1L));
	if ( wdallocator ) {
		Initialize();	// used to initialize gsAllocatorInit Mutex
		// in mt case we need to control ref counting with mutexes
		MutexEntry me(*gsAllocatorInit);
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
	// silently ignore misuse of this api
}

void MT_Storage::UnrefAllocator(Allocator *wdallocator)
{
	StartTrace1(MT_Storage.UnrefAllocator, "Id:" << (wdallocator ? wdallocator->GetId() : -1L));
	if (wdallocator) {	// just to be robust wdallocator == 0 should not happen
		Initialize();	// used to initialize gsAllocatorInit Mutex
		MutexEntry me(*gsAllocatorInit);
		me.Use();
		wdallocator->Unref();
		Trace("refcount is now:" << wdallocator->RefCnt());

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
	// silently ignore misuse of this api
}

bool MT_Storage::RegisterThread(Allocator *wdallocator)
{
	StartTrace(MT_Storage.RegisterThread);
	// can only be used once for the same thread

	Allocator *oldAllocator = 0;

	// determine which allocator to use
	FindAllocator(oldAllocator);
	if (!oldAllocator) {
		return ! SETTLSDATA(MT_Storage::fgAllocatorKey, wdallocator);
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

void MTStorageHooks::Finalize()
{
	Initialize();	// used to initialize gsAllocatorInit Mutex
	// terminate pool allocators
	MutexEntry me(*gsAllocatorInit);
	me.Use();
	while (fgPoolAllocatorList) {
		AllocList *elmt = fgPoolAllocatorList;
		delete elmt->wdallocator;
		fgPoolAllocatorList = elmt->next;
		::free(elmt);
	}
	Storage::DoFinalize();
}

Allocator *MTStorageHooks::Global()
{
	return 	Storage::DoGlobal();
}

#ifdef MEM_DEBUG
MemTracker *MTStorageHooks::MakeMemTracker(const char *name)
{
	return new MT_MemTracker(name);
}
#endif

Allocator *MTStorageHooks::Current()
{
	Allocator *wdallocator = 0;

	// Storage::Current() might be called before Storage::Global()
	// therefore need to initialize here too
	Storage::Initialize();

	// determine which allocator to use
	FindAllocator(wdallocator);

	if (wdallocator) {
		return wdallocator;
	}

	return Storage::DoGlobal();
}

void MTStorageHooks::Initialize()
{
	if ( !fgInitialized ) { // need mutex??? not yet, we do not run MT
		// setup key used to store allocator in thread local storage
		if (THRKEYCREATE(MT_Storage::fgAllocatorKey, TLSDestructor)) {
			SysLog::Error("could not create TLS key");
		}
		fgInitialized = true;
		Storage::DoInitialize();
	}
}
