/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Queue_H
#define _Queue_H

//---- baseclass include -------------------------------------------------
#include "config_Queueing.h"
#include "STLStorage.h"
#include "Threads.h"
#include "DateTime.h"

#include <limits.h>

//---- Queue ----------------------------------------------------------
//! <B>Simple, thread-safe, container based queue</B>
/*!
Queue elements are represented using either by objects of their type or Anythings. The internal queue itself is either a std::container or an Anything which allows
simple handling.
Statistics can be made by evaluating the returned Anything from GetStatistics(). The following slots are defined:
<pre>
{
	/QueueSize		long		size of the queue
	/CurrentSize	long		current element count in queue
	/MaxLoad		long		maximum number of entries
	/CreateTime {				time when Q was created
		/sec		long		seconds since 1.1.1970
		/usec		long		microseconds of current second
	}
	/StatisticTime {			time when statistics were queried
		/sec		long		seconds since 1.1.1970
		/usec		long		microseconds of current second
	}
	/LifeTime {					life time of the queue
		/sec		long		seconds difference
		/usec		long		microseconds difference
	}
	/PutCount		long		how many elements were put since queue was instantiated
	/GetCount		long		how many elements were get since queue was instantiated
}
</pre>
<b>* marks slots not yet available</b>
*/
template <
class TElementType,
	  class TListStorageType
	  >
class QueueBase : public IFAObject
{
	friend class QueueTest;
public:
	typedef TElementType ElementType;
	typedef ElementType &ElementTypeRef;
	typedef TListStorageType ListStorageType;
	typedef ListStorageType &ListStorageTypeRef;
	typedef QueueBase<ElementType, ListStorageType> ThisType;

	//--- constructors
	QueueBase(const char *name, long lQueueSize = LONG_MAX, Allocator *pAlloc = Storage::Global())
		: fName(name, -1, pAlloc)
		, fAllocator(pAlloc)
		, fSemaFullSlots(0L)
		, fSemaEmptySlots(lQueueSize)
		, fPutCount(0L)
		, fGetCount(0L)
		, fMaxLoad(0L)
		, fBlockingPutCount(0L)
		, fBlockingGetCount(0L)
		, fAlive(0xf007f007)
		, feBlocked(eBothSides)
		, fQueueLock("QueueMutex", fAllocator)
		, fBlockedLock("BlockedLock", fAllocator)
		, fBlockingPutLock("BlockingPutLock", fAllocator)
		, fBlockingGetLock("BlockingGetLock", fAllocator)
		, fContainer()
		, fAnyStatistics(fAllocator) {
		StartTrace1(Queue.Queue, "queue size:" << lQueueSize);
		fAnyStatistics["QueueSize"] = lQueueSize;
		Anything anyTime;
		DateTime::GetTimeOfDay(anyTime);
		fAnyStatistics["CreateTime"]["sec"] = anyTime["sec"].AsLong(0L);
		fAnyStatistics["CreateTime"]["usec"] = anyTime["usec"].AsLong(0L);
		UnBlock();
	}

	~QueueBase() {
		StartTrace(Queue.~Queue);
		// block any pending interface accesses
		Block();

		LockUnlockEntry me(fQueueLock);
		// mark that we are no longer alive
		fAlive = 0x00dead00;

		long lSize(IntGetSize());
		if ( lSize > 0L ) {
			SYSWARNING("Destruction of non-empty queue [" << fName << "]! still " << lSize << " Elements in Queue!");
			ListStorageType anyElements;
			IntEmptyQueue(anyElements);
		}
	}

	/*! Status code of queue Put and Get operations. As we support blocking of either read and/or write side of the queue, we must return appropriate codes telling the caller what happened. A simple boolean value is far not enough to find out why an operation failed. */
	enum StatusCode {
		eSuccess = 0,								//! put or get was successful
		eEmpty = 1,									//! the queue did not contain any elements when accessing it
		eFull = eEmpty << 1,						//! the queue was already full when trying to put an element
		eBlocked = eFull << 1,
		eError = eBlocked << 1,
		eAcquireFailed = eError << 1,
		eTryAcquireFailed = eAcquireFailed << 1,
		eDead = eTryAcquireFailed << 1,
	};
	StatusCode Put(ElementTypeRef anyElement, bool bTryLock = false) {
		StartTrace(Queue.Put);
		StatusCode eRet(eBlocked);
		if ( !IsBlocked(ePutSide) ) {
			if ( bTryLock ) {
				eRet = /* eTryAcquireFailed |*/ eFull;
				if ( fSemaEmptySlots.TryAcquire() ) {
					eRet = DoPut(anyElement);
				}
			} else {
				LockedValueIncrementDecrementEntry ce(fBlockingPutLock, fBlockingPutCond, fBlockingPutCount);
				// need double checking here because of possible race condition during destruction on fBlockingPutLock
				if ( IsAlive() && fSemaEmptySlots.Acquire() ) {
					eRet = eDead;
					if ( IsAlive() ) {
						eRet = DoPut(anyElement);
					} else {
						// must release semaphore again because we did not put an element
						fSemaEmptySlots.Release();
					}
				}
			}
		}
		return eRet;
	}

	StatusCode Get(ElementTypeRef anyElement, bool bTryLock = false) {
		StartTrace(Queue.Get);
		StatusCode eRet(eBlocked);
		if ( !IsBlocked(eGetSide) ) {
			if ( bTryLock ) {
				eRet = /*eTryAcquireFailed |*/ eEmpty;
				if ( fSemaFullSlots.TryAcquire() ) {
					eRet = DoGet(anyElement);
				}
			} else {
				LockedValueIncrementDecrementEntry ce(fBlockingGetLock, fBlockingGetCond, fBlockingGetCount);
				// need double checking here because of possible race condition during destruction on fBlockingGetLock
				eRet = eAcquireFailed;
				if ( IsAlive() && fSemaFullSlots.Acquire() ) {
					eRet = eDead;
					if ( IsAlive() ) {
						eRet = DoGet(anyElement);
					} else {
						// must release semaphore again because we did not get an element
						fSemaFullSlots.Release();
					}
				}
			}
		}
		return eRet;
	}

	template < class DestListType >
	void EmptyQueue(DestListType &anyElements) {
		StartTrace(Queue.EmptyQueue);
		if ( !IsBlocked() ) {
			LockUnlockEntry me(fQueueLock);
			this->IntEmptyQueue(anyElements);
		}
	}
	long GetSize() {
		StartTrace(Queue.GetSize);
		long lSize = -1;
		if ( !IsBlocked() ) {
			LockUnlockEntry me(fQueueLock);
			lSize = IntGetSize();
		}
		return lSize;
	}

	void GetStatistics(Anything &anyStatistics) {
		StartTrace(Queue.GetStatistics);
		if ( IsAlive() ) {
			long lCurSec(0L), lCurUSec(0L);
			{
				LockUnlockEntry me(fQueueLock);
				// must pass a DeepClone because of reference counting
				anyStatistics = fAnyStatistics.DeepClone(anyStatistics.GetAllocator());
				// this Anything uses Storage::Current as default allocator so we must take care
				// when we use a Thread specific PoolAllocator because the Pool access is not locked
				Anything anyTime;
				DateTime::GetTimeOfDay(anyTime);
				lCurSec = anyTime["sec"].AsLong(0L);
				lCurUSec = anyTime["usec"].AsLong(0L);
			}
			anyStatistics["MaxLoad"] = fMaxLoad;
			anyStatistics["PutCount"] = (long)fPutCount;
			anyStatistics["GetCount"] = (long)fGetCount;
			anyStatistics["CurrentSize"] = IntGetSize();
			anyStatistics["StatisticTime"]["sec"] = lCurSec;
			anyStatistics["StatisticTime"]["usec"] = lCurUSec;

			long lSecDiff(lCurSec - anyStatistics["CreateTime"]["sec"].AsLong());
			long lUSecDiff(lCurUSec - anyStatistics["CreateTime"]["usec"].AsLong());
			if ( lUSecDiff < 0L ) {
				--lSecDiff;
				lUSecDiff += 1000000L;
			}
			anyStatistics["LifeTime"]["sec"] = lSecDiff;
			anyStatistics["LifeTime"]["usec"] = lUSecDiff;

			TraceAny(anyStatistics, "statistics");
		}
	}

	inline bool IsAlive() {
		return fAlive == 0xf007f007;
	}

	enum BlockingSide {
		eNone = 0,
		ePutSide = 1,
		eGetSide = ePutSide << 1,
		eBothSides = ePutSide | eGetSide,
	};

	bool IsBlocked(BlockingSide aSide = eBothSides) {
		StartTrace(Queue.IsBlocked);
		if ( IsAlive() ) {
			// need only a read lock here
			LockUnlockEntry rwe(fBlockedLock, RWLock::eReading);
			return ( ( feBlocked & aSide ) == aSide );
		}
		return true;
	}

	void Block(BlockingSide aSide = eBothSides) {
		StartTrace1(Queue.Block, "side:" << aSide);
		if ( IsAlive() ) {
			{
				// need a write lock here
				LockUnlockEntry rwe(fBlockedLock, RWLock::eWriting);
				feBlocked = ( BlockingSide )( feBlocked | ( aSide & eBothSides ) );
			}
			// release potentially waiting putters/getters
			LockUnlockEntry me(fQueueLock);
			if ( aSide & ePutSide ) {
				IntReleaseBlockedPutters();
			}
			if ( aSide & eGetSide ) {
				IntReleaseBlockedGetters();
			}
		}
	}

	void UnBlock(BlockingSide aSide = eBothSides) {
		StartTrace1(Queue.UnBlock, "side:" << aSide);
		if ( IsAlive() ) {
			// need a write lock here
			LockUnlockEntry rwe(fBlockedLock, RWLock::eWriting);
			feBlocked = ( BlockingSide )( feBlocked & ( ~aSide & eBothSides ) );
		}
	}

	virtual IFAObject *Clone() const {
		return NULL;
	}

protected:
	StatusCode DoPut(ElementTypeRef anyElement) {
		StartTrace(Queue.DoPut);
		StatusCode eRet(eBlocked);
		if ( !IsBlocked(ePutSide) ) {
			LockUnlockEntry me(fQueueLock);
			fContainer.push_back(anyElement);
			++fPutCount;
			fMaxLoad = std::max( fMaxLoad, (long)fContainer.size() );
			fSemaFullSlots.Release();
			eRet = eSuccess;
		}
		return eRet;
	}

	StatusCode DoGet(ElementTypeRef anyElement) {
		StartTrace(Queue.DoGet);
		StatusCode eRet(eBlocked);
		if ( !IsBlocked(eGetSide) ) {
			LockUnlockEntry me(fQueueLock);
			if ( fContainer.size() ) {
				anyElement = fContainer.front();	//TODO - future: change here so that it's not restricted only for Anything's - use any_cast
				fContainer.pop_front();
				++fGetCount;
				fSemaEmptySlots.Release();
				eRet = eSuccess;
			} else {
				// we can only get here if something curious happens with semaphore handling
				// because we were able to acquire a full-slot just before, something must have happened at this point
				// which should not be possible by its design
				// -> someone is destructing the queue without blocking first?
				SYSERROR("accessed empty Queue!?");
				eRet = ( StatusCode )( eEmpty | eError );
			}
		}
		return eRet;
	}

	long IntGetSize() {
		StartTrace(Queue.IntGetSize);
		long lRet(fContainer.size());
		Trace("Anything  size:" << lRet);
		Trace("(Put-Get) size:" << (long)(fPutCount - fGetCount));
		return lRet;
	}

	template < class DestListType >
	void IntEmptyQueue(DestListType &anyElements) {
		StartTrace(Queue.IntEmptyQueue);
		long lSize(IntGetSize());
		Trace("current queue size:" << lSize);
		// try to optimize by using swap to exchange internal with external list
		// => NO, it would copy the allocator too, which is dangerous in case we use PoolAllocators belonging to threads
		while ( --lSize >= 0 ) {
			fSemaFullSlots.TryAcquire();
			moveElement(fContainer, anyElements);
			fSemaEmptySlots.Release();
		}
		Trace("elements removed:" << (long)anyElements.size());
	}

	template < class DestListType >
	void moveElement(ListStorageTypeRef aFrom, DestListType &aTo) {
		aTo.push_back(aFrom.front());
		aFrom.pop_front();
	}

	void IntReleaseBlockedPutters() {
		StartTrace(Queue.IntReleaseBlockedPutters);
		LockUnlockEntry sme(fBlockingPutLock);
		if ( fBlockingPutCount ) {
			SYSWARNING("waking up " << fBlockingPutCount << " blocked Put() callers");
		}
		while ( fBlockingPutCount ) {
			Trace("releasing one of " << fBlockingPutCount << " blocked Putters");
			fSemaEmptySlots.Release();
			// allow signalled thread to decrement the fBlockingPutCount after failed return from Put
			fBlockingPutCond.Wait(fBlockingPutLock);
		}
	}

	void IntReleaseBlockedGetters() {
		StartTrace(Queue.IntReleaseBlockedGetters);
		LockUnlockEntry sme(fBlockingGetLock);
		if ( fBlockingGetCount ) {
			SYSINFO("waking up " << fBlockingGetCount << " blocked Get() callers");
		}
		while ( fBlockingGetCount ) {
			Trace("releasing one of " << fBlockingGetCount << " blocked Getters");
			fSemaFullSlots.Release();
			// allow signalled thread to decrement the fBlockingGetCount after failed return from Get
			fBlockingGetCond.Wait(fBlockingGetLock);
		}
	}

	String		fName;
	Allocator	*fAllocator;
	Semaphore	fSemaFullSlots, fSemaEmptySlots;
	ul_long		fPutCount, fGetCount;
	long		fMaxLoad, fBlockingPutCount, fBlockingGetCount;
	u_long		fAlive;
	BlockingSide feBlocked;
	Mutex		fQueueLock;
	RWLock		fBlockedLock;
	SimpleMutex fBlockingPutLock, fBlockingGetLock;
	SimpleMutex::ConditionType fBlockingPutCond, fBlockingGetCond;
	ListStorageType fContainer;
	Anything	fAnyStatistics;
};

template <
class TElementType,
	  class TListStorageType
	  >
class Queue : public QueueBase<TElementType, TListStorageType>
{
	friend class QueueTest;
public:
	typedef TElementType ElementType;
	typedef ElementType &ElementTypeRef;
	typedef TListStorageType ListStorageType;
	typedef ListStorageType &ListStorageTypeRef;
	typedef QueueBase<ElementType, ListStorageType> BaseType;
	typedef Queue<ElementType, ListStorageType> ThisType;

	Queue(const char *name, long lQueueSize = LONG_MAX, Allocator *pAlloc = Storage::Global())
		: BaseType(name, lQueueSize, pAlloc) {
		StatTrace(Queue.Queue, "generic", Storage::Current());
	}
};

template <
class TElementType
>
class Queue<TElementType, Anything> : public QueueBase<TElementType, Anything>
{
	friend class QueueTest;
public:
	typedef TElementType ElementType;
	typedef ElementType &ElementTypeRef;
	typedef Anything ListStorageType;
	typedef ListStorageType &ListStorageTypeRef;
	typedef QueueBase<ElementType, ListStorageType> BaseType;
	typedef Queue<ElementType, ListStorageType> ThisType;

	Queue(const char *name, long lQueueSize = LONG_MAX, Allocator *pAlloc = Storage::Global())
		: BaseType(name, lQueueSize, pAlloc) {
		StatTrace(Queue.Queue, "Anything", Storage::Current());
		this->fContainer.SetAllocator(pAlloc);
	}
};

typedef Queue<Anything, Anything> AnyQueueType;

#endif
