/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ITOStorage_H
#define _ITOStorage_H

#include <sys/types.h>
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "config.h"				// for definition of own types

#if !defined(__370__) && defined(DEBUG)
#define MEM_DEBUG
#endif

#ifdef MEM_DEBUG

//! Base class for memory allocation tracking
/*! helper class for debugging memory management problems */
class EXPORTDECL_FOUNDATION MemTracker
{
public:
	MemTracker(const char *name);
	virtual ~MemTracker();

	//!tracks allocation; chunk allocated has allocSz
	virtual void TrackAlloc(u_long allocSz);

	//!tracks free; chunk freed has allocSz
	virtual void TrackFree(u_long allocSz);

	//!prints statistic of allocated and freed bytes
	virtual void PrintStatistic();

	//!returns currently allocated bytes
	l_long CurrentlyAllocated() {
		return fAllocated;
	}

	//!returns peak allocated bytes
	l_long PeakAllocated() {
		return fMaxAllocated;
	}

	//!sets the id of the allocator to be tracked
	void SetId(long);

	//!initializes statistics to the values of MemTracker t
	virtual void Init(MemTracker *t);

protected:
	//!tracks the currently allocated size in bytes, and the peek allocated size
	l_long  fAllocated, fMaxAllocated;
	//!tracks the number and maximum of allocated bytes
	l_long  fNumAllocs, fSizeAllocated;
	//!tracks the number and maximum of freed bytes
	l_long  fNumFrees, fSizeFreed;

public:
	//! the id of the pool we track
	long fId;
	//! the name of the pool we track
	const char *fpName;
};

//!helper class to check for memory leaks
class EXPORTDECL_FOUNDATION MemChecker
{
public:
	//! check allocation in in a certain scope for one allocator
	//! we do not use String on purpose here
	MemChecker(const char *scope, class Allocator *a);
	~MemChecker();

	//! print delta bytes of allocation since the constructor was run on cerr
	void TraceDelta(const char *message);

	//! returns delta bytes of allocation since the constructor was run
	l_long CheckDelta();
	void Use() { }

protected:
	//!the allocator which is checked
	Allocator *fAllocator;

	//!the number of bytes allocated minus number of bytes freed
	l_long fSizeAllocated;

	//!message to identify the scope the check was executed
	const char *fScope;
};

#define StartTraceMem(scope) MemChecker rekcehc(_QUOTE_(scope), Storage::Current())
#define TraceMemDelta(message) rekcehc.TraceDelta(message)

#define StartTraceMem1(scope, allocator) MemChecker rekcehc1(_QUOTE_(scope), allocator)
#define TraceMemDelta1(message) rekcehc1.TraceDelta(message)

#define MemTrackDef(tracker) MemTracker tracker;
#define MemTrackInit(tracker) , tracker(#tracker)
#define MemTrackAlloc(tracker, allocSz) tracker.TrackAlloc(allocSz)
#define MemTrackFree(tracker, allocSz) tracker.TrackFree(allocSz)
#define MemTrackStat(tracker) tracker.PrintStatistic()
#define MemTrackStatIfAllocated(tracker) if ( tracker.PeakAllocated() > 0 ) tracker.PrintStatistic()
#define MemTrackStillAllocated(tracker)	( tracker.CurrentlyAllocated() > 0 )
#define MemTrackStillAllocatedException(tracker, where)	if( tracker.CurrentlyAllocated() > 0 )	\
	{	\
		SysLog::Error(String("PoolAllocator was still in use! (id: ", Storage::Global()) << tracker.fId << " name [" << NotNull(tracker.fpName) << "]) in " << where); \
	}
#define PoolTrackStat(pAlloc) pAlloc->PrintStatistic()
#define PoolTrackStatTriggered(trigger, pAlloc)	\
		if ( TraceTriggered(trigger, Storage::Global()) )	\
		{\
			 pAlloc->PrintStatistic();	\
		}

#else
#define StartTraceMem(scope)
#define StartTraceMem1(scope, allocator)
#define TraceMemDelta(message)
#define TraceMemDelta1(message)

#define MemTrackDef(tracker)
#define MemTrackInit(tracker)
#define MemTrackAlloc(tracker, allocSz)
#define MemTrackFree(tracker, allocSz)
#define MemTrackStat(tracker)
#define MemTrackStatIfAllocated(tracker)
#define MemTrackStillAllocated(tracker)	false
#define MemTrackStillAllocatedException(tracker, where)

#define PoolTrackStat(tracker)
#define PoolTrackStatTriggered(trigger, pAlloc)
#endif

class EXPORTDECL_FOUNDATION MemoryHeader;

//! Base class for memory allocation policies
class EXPORTDECL_FOUNDATION Allocator
{
public:
	Allocator(long allocatorid);
	virtual ~Allocator();

	//!analogous api to built in c function calloc
	void *Calloc(int n, size_t size);

	//!analogous api to built in c function malloc
	void *Malloc(size_t size);

	//!analogous api to built in c function free
	virtual void  Free(void *vp) = 0;

	//!refcounting support
	void Ref() 		{
		fRefCnt++;
	}
	//!refcounting support
	void Unref() 	{
		fRefCnt--;
	}
	//!refcounting support
	virtual long RefCnt()	{
		return fRefCnt;
	}

	//! hook to allow allocators to optimize allocation of string buffers
	virtual size_t SizeHint(size_t size) {
		return size;
	}

	/*! set an identification for this pool
		\param lId identification for pool
		\return old identifier */
	virtual long SetId(long lId) {
		long lOldId = fAllocatorId;
		fAllocatorId = lId;
		return lOldId;
	}

	/*! get identifier of this pool
		\return identifier of this pool */
	long GetId() {
		return fAllocatorId;
	}

#ifdef MEM_DEBUG
	//!Memory debugging and tracking support; implementer should report currently allocated bytes
	virtual l_long CurrentlyAllocated() = 0;
	//!change of memtrackers to be e.g. MT-Safe
	virtual void ReplaceMemTracker(MemTracker *t) {}
#endif
	//!Memory debugging and tracking support; implementer should report all statistic on cerr
	virtual void PrintStatistic() = 0;

	//!hook method to reorganize the managed memory
	virtual void Refresh();

protected:
	//!hook for allocation of memory
	virtual void *Alloc(u_long allocSize) = 0;

	//!calculates the memory needed
	//! \param n number of objects needed
	//! \param size size of the object
	virtual u_long AllocSize(u_long n, size_t size);

	//!calculates the offset into a buffer from the beginning of a managed piece of memory; clients should not care about the memory header before ExtMemStart
	virtual void *ExtMemStart(void *vp);

	//!calculates the real start of the memory managed from vp, assuming it contains a MemoryHeader
	virtual MemoryHeader *RealMemStart(void *vp);

	//!identification of the allocater
	long fAllocatorId;

	//!reference count
	long fRefCnt;
};

//!manages storage using the builtin c api and does some statistic
class EXPORTDECL_FOUNDATION GlobalAllocator: public Allocator
{
public:
	//!does nothing
	GlobalAllocator();

	//!prints only statistics
	virtual ~GlobalAllocator();

	//!frees memory allocated by global allocator
	virtual void  Free(void *vp);

	//!reference counting is disabled global allocator always exists (but only once)
	virtual long RefCnt()	{
		return 1;
	}

	//!print out the allocators statistics (only available if MEM_DEBUG is enabled)
	virtual void PrintStatistic();
#ifdef MEM_DEBUG
	//!returns the currently allocated bytes (only available if MEM_DEBUG is enabled)
	l_long CurrentlyAllocated();
	//!replaces the memory tracker with sthg. different e.g. thread safe  (only available if MEM_DEBUG is enabled)
	virtual void ReplaceMemTracker(MemTracker *t);
#endif
protected:
	//!implements allocation bottleneck routine
	virtual void *Alloc(u_long allocSize);
#ifdef MEM_DEBUG
	friend class MemChecker;

	//!tracks allocation and deallocation of memory
	MemTracker *fTracker;
#endif
};

//!wrapper class to provide protocol for dispatching if non standard (GlobalAllocator) is used
struct EXPORTDECL_FOUNDATION StorageHooks {
	//!initialize storage subsystem
	virtual void Initialize() = 0;
	//!finalize storage subsystem
	virtual void Finalize() = 0;
	//!access global allocator
	virtual Allocator *Global() = 0;
	//!access allocator set for current context (e.g. thread)
	virtual Allocator *Current() = 0;
#ifdef MEM_DEBUG
	//!allocate a memory tracker object (only available if MEM_DEBUG is set)
	virtual MemTracker *MakeMemTracker(const char *name) = 0;
#endif
};

//! a wrapper for memory allocations and deallocations that allows to dispatch memory management to allocator policies
//!a wrapper for basic memory management functions. This allows dispatching of memory management to specialized allocator objects.
class EXPORTDECL_FOUNDATION Storage
{
public:
	//!initalize memory management depending on memory management strategy set
	static void Initialize();

	//!cleanup memory management
	static void Finalize();

	//!prints memory management statistics
	static void PrintStatistic();
#ifdef MEM_DEBUG
	//!factory method to allocate memory management specific MemTracker
	static MemTracker *MakeMemTracker(const char *name);
#endif
	//! get the global allocator
	static Allocator *Global();

	//! get currently used allocator (thread specific)
	static Allocator *Current();

protected:
	// somewhat silly the need to declare all storage hook derivations as friend
	friend class MT_Storage;
	friend class MTStorageHooks;
	friend class TestStorageHooks;
	friend class Server;
	friend class SessionTest;
	friend class FinalCleaner;
	friend class SocketTest;
	friend class SSLConnectorTest;
	friend class ConnectorTest;
	friend class AnythingPerfTest;

	//! used by mt system to redefine the hooks for mt-local storage policy
	static void SetHooks(StorageHooks *h) {
		fgHooks = h;
	}

	//!temporarily disable thread local storage policies e.g. to reinitialize server
	static void ForceGlobalStorage(bool b) {
		fgForceGlobal = b;
	}

	//!access the global allocator
	static Allocator *DoGlobal();

	//!do nothing; print statistics
	static void DoInitialize();

	//!do nothing; print statistics
	static void DoFinalize();
#ifdef MEM_DEBUG
	//!factory method to allocate MemTracker
	static MemTracker *DoMakeMemTracker(const char *name);
#endif

	//!exchange this object when MT_Storage is used
	static struct StorageHooks *fgHooks;

	//!flag to force global store temporarily
	static bool fgForceGlobal;

	//!MT_Storage__Global needs access to this class
	friend class EXPORTDECL_FOUNDATION Allocator *MT_Storage__Global();
	//! the global allocator
	static Allocator *fgGlobalPool;
};

class EXPORTDECL_FOUNDATION TestStorageHooks : public StorageHooks
{
public:
	TestStorageHooks(Allocator *allocator);
	virtual void Initialize();
	virtual void Finalize();
	virtual Allocator *Global();
	virtual Allocator *Current();
#ifdef MEM_DEBUG
	virtual MemTracker *MakeMemTracker(const char *name);
#endif

	Allocator *fAllocator;
	static bool fgInitialized;
};

#endif		//not defined _ITOStorage_H
