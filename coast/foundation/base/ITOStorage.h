/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ITOStorage_H
#define _ITOStorage_H

#include "foundation.h"			// for definition of own types
#include <sys/types.h>
#include <iosfwd>
#include <cstdlib>
#include <deque>

class MemoryHeader;

namespace ITOStorage {

	// adaption of Storage::Global / Storage::Current for boost::poolXXX usage
	struct BoostPoolUserAllocatorGlobal {
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		static char *malloc(const size_type bytes);
		static void free(char *const block);
	};

	struct BoostPoolUserAllocatorCurrent {
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		static char *malloc(const size_type bytes);
		static void free(char *const block);
	};
}

//! Base class for memory allocation tracking
/*! helper class for debugging memory management problems */
class MemTracker
{
	friend class MemoryHeader;
	friend class MemTrackerTest;
public:
	typedef std::deque<MemoryHeader *> UsedListType;
	typedef UsedListType *UsedListTypePtr;

	MemTracker(const char *name);
	virtual ~MemTracker();

	//!tracks allocation; chunk allocated has allocSz
	virtual void TrackAlloc(MemoryHeader *mh);

	//!tracks free; chunk freed has allocSz
	virtual void TrackFree(MemoryHeader *mh);

	//! print out still allocated memory blocks
	void DumpUsedBlocks();

	//!prints statistic of allocated and freed bytes
	virtual void PrintStatistic(long lLevel);

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

	//! gets the Trackers/Allocators id
	long GetId() {
		return fId;
	}

	//! returns the name of the Tracker
	const char *GetName() {
		return fpName;
	}

	static void *operator new(size_t size);
	static void *operator new(size_t size, class Allocator *);
	static void operator delete(void *d);

protected:
	//!tracks the currently allocated size in bytes, and the peek allocated size
	l_long  fAllocated, fMaxAllocated;
	//!tracks the number and maximum of allocated bytes
	l_long  fNumAllocs, fSizeAllocated;
	//!tracks the number and maximum of freed bytes
	l_long  fNumFrees, fSizeFreed;

private:
	MemTracker();
	MemTracker(const MemTracker &);
	MemTracker &operator=(const MemTracker &);

	//! the id of the pool we track
	long fId;
	//! the name of the pool we track
	const char *fpName;
	//! list to store used MemoryHeaders
	UsedListTypePtr fpUsedList;
};

//!helper class to check for memory leaks
class MemChecker
{
	MemChecker(const MemChecker &);
	MemChecker &operator=(const MemChecker &);
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

#define StartTraceMem(scope) MemChecker rekcehc(_QUOTE_(scope), Coast::Storage::Current())
#define TraceMemDelta(message) rekcehc.TraceDelta(message)

#define StartTraceMem1(scope, allocator) MemChecker rekcehc1(_QUOTE_(scope), allocator)
#define TraceMemDelta1(message) rekcehc1.TraceDelta(message)

#define PoolTrackStatTriggered(trigger, pAlloc, level) if ( TriggerEnabled(trigger) ) { pAlloc->PrintStatistic(level); }

//! Base class for memory allocation policies
class Allocator
{
public:
	Allocator(long allocatorid);
	virtual ~Allocator();

	//!analogous api to built in c function calloc
	void *Calloc(int n, size_t size);

	template<size_t SIZE>
	void *Malloc() {
		return Malloc(SIZE);
	}

	template<size_t SIZE>
	void Free(void *p) {
		Free(p, SIZE);
	}

	//!analogous api to built in c function malloc
	virtual void *Malloc(size_t size);

	//!analogous api to built in c function free
	virtual void Free(void *vp, size_t sz) = 0;

	//!analogous api to built in c function free
	virtual void Free(void *vp) = 0;

	//!refcounting support
	virtual void Ref() 		{
		++fRefCnt;
	}
	//!refcounting support
	virtual void Unref() 	{
		--fRefCnt;
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
	virtual long GetId() {
		return fAllocatorId;
	}

	//!Memory debugging and tracking support; implementer should report currently allocated bytes
	virtual l_long CurrentlyAllocated() = 0;

	//!change of memtrackers to be e.g. MT-Safe
	virtual MemTracker *ReplaceMemTracker(MemTracker *t) {
		return NULL;
	}

	//!Memory debugging and tracking support; implementer should report all statistic on cerr
	virtual void PrintStatistic(long lLevel = -1) = 0;

	//!hook method to reorganize the managed memory
	virtual void Refresh();

protected:
	//!hook for allocation of memory
	virtual void *Alloc(size_t allocSize) = 0;

	//TODO The methods AllocSize, ExtMemStart and RealMemStart would be better moved to MemoryHeader

	//!calculates the memory needed
	//! \param n number of objects needed
	//! \param size size of the object
	virtual size_t AllocSize(size_t n, size_t size);

	//!calculates the offset into a buffer from the beginning of a managed piece of memory; clients should not care about the memory header before ExtMemStart
	virtual void *ExtMemStart(void *vp);

	//!calculates the real start of the memory managed from vp, assuming it contains a MemoryHeader
	virtual MemoryHeader *RealMemStart(void *vp);

	//!identification of the allocator
	long fAllocatorId;

	//!reference count
	long fRefCnt;

	//!tracks allocation and deallocation of memory
	MemTracker *fTracker;
};

//!manages storage using the built-in C API and does some statistic
class GlobalAllocator: public Allocator
{
	GlobalAllocator(const GlobalAllocator &);
	GlobalAllocator &operator=(const GlobalAllocator &);
public:
	//!does nothing
	GlobalAllocator();

	//!prints only statistics
	virtual ~GlobalAllocator();

	//!frees memory allocated by global allocator
	virtual void Free(void *vp);

	//!frees memory allocated by global allocator
	virtual void Free(void *vp, size_t sz);

	//!reference counting is disabled global allocator always exists (but only once)
	virtual long RefCnt()	{
		return 1;
	}

	//!print out the allocators statistics
	virtual void PrintStatistic(long lLevel = -1);

	//!returns the currently allocated bytes
	l_long CurrentlyAllocated();

	//!replaces the memory tracker with sthg. different e.g. thread safe
	virtual MemTracker *ReplaceMemTracker(MemTracker *t);

protected:
	//!implements allocation bottleneck routine
	virtual void *Alloc(size_t allocSize);
	friend class MemChecker;
};

//!wrapper class to provide protocol for dispatching if non standard (GlobalAllocator) is used
class StorageHooks
{
	StorageHooks(const StorageHooks &);
	StorageHooks &operator=(const StorageHooks &);
public:
	StorageHooks()
		: fpOldHook(0) {};

	virtual ~StorageHooks() {
		fpOldHook = 0;
	};

	//!initialize storage subsystem
	virtual void Initialize() = 0;

	//!finalize storage subsystem
	virtual void Finalize() = 0;

	//!access global allocator
	virtual Allocator *Global() = 0;

	//!access allocator set for current context (e.g. thread)
	virtual Allocator *Current() = 0;

	/*! allocate a memory tracker object
		\param name name of the tracker
		\param bThreadSafe specify if tracker must be thread safe or not - not used from within foundation
		\return poniter to a newly created MemTracker object */
	virtual MemTracker *MakeMemTracker(const char *name, bool bThreadSafe) = 0;

	void SetOldHook(StorageHooks *pOld) {
		fpOldHook = pOld;
	}

	StorageHooks *GetOldHook() {
		return fpOldHook;
	}
	virtual void Lock() {}
	virtual void Unlock() {}

	template <typename T>
    class LockingProxy {
    public:
    	LockingProxy(T *t, StorageHooks *pH): fT(t), fHooks(pH) {
    		fHooks->Lock();
    	};

    	T* operator->() const {
    		return fT;
    	}

    	~LockingProxy() {
    		fHooks->Unlock();
    	}
    private:
    	T* fT;
    	StorageHooks* fHooks;
    };

    template<typename T>
    LockingProxy<T> LockedAccessFor(const T& t) {
    	return LockingProxy<T>(&t, this);
    }

private:
	StorageHooks *fpOldHook;
};

class FoundationStorageHooks: public StorageHooks {
	bool fgInitialized;
public:
	FoundationStorageHooks() :
		fgInitialized(false) {
	}
	virtual void Initialize();
	virtual void Finalize();
	virtual Allocator *Global();
	virtual Allocator *Current();
	virtual MemTracker *MakeMemTracker(const char *name, bool);
};

namespace Coast
{
	namespace Memory
	{
		template<typename T>
		struct AlignedSize {
			static const size_t value = sizeof(T) +
					( sizeof(T) % sizeof(long double) ?
							(sizeof(long double) - sizeof(T) % sizeof(long double)) : 0);
		};

		Allocator*& allocatorFor(void* ptr);

		void *realPtrFor(void *ptr);

		void safeFree(Allocator *a, void *ptr);
	} // namespace Memory

	namespace Storage
	{
		//!initalize memory management depending on memory management strategy set
		void Initialize();

		//!cleanup memory management
		void Finalize();

		//!prints memory management statistics
		void PrintStatistic(long lLevel = -1);

		/*! allocate a memory tracker object
			\param name name of the tracker
			\param bThreadSafe specify if tracker must be thread safe or not - not used from within foundation
			\return poniter to a newly created MemTracker object */
		MemTracker *MakeMemTracker(const char *name, bool bThreadSafe);

		//! get the global allocator
		Allocator *Global();

		//! get currently used allocator (thread specific)
		Allocator *Current();

		/*! get statistic level which was initialized by getting value of COAST_TRACE_STORAGE environment variable
			\return logging level, see description of fglStatisticLevel */
		long GetStatisticLevel();

		//! used by mt system to redefine the hooks for mt-local storage policy
		StorageHooks *SetHooks(StorageHooks *h);

		//!temporarily disable thread local storage policies e.g. to reinitialize server
		void ForceGlobalStorage(bool b);

		//!access the global allocator
		Allocator *DoGlobal();

		//!do nothing; print statistics
		void DoInitialize();

		//!do nothing; print statistics
		void DoFinalize();

		//!factory method to allocate MemTracker
		MemTracker *DoMakeMemTracker(const char *name);
	} // namespace Storage
} // namespace Coast


class TestStorageHooks : public StorageHooks
{
	TestStorageHooks(const TestStorageHooks &);
	TestStorageHooks &operator=(const TestStorageHooks &);
public:
	TestStorageHooks(Allocator *allocator);
	virtual ~TestStorageHooks();
	virtual void Initialize();
	virtual void Finalize();
	virtual Allocator *Global();
	virtual Allocator *Current();
	virtual MemTracker *MakeMemTracker(const char *name, bool);

	Allocator *fAllocator;
	StorageHooks *fpOldHook;
};

#endif		//not defined _ITOStorage_H
