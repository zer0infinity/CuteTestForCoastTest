/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Queue.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SysLog.h"
#include "DateTime.h"

//--- c-modules used -----------------------------------------------------------

class CountEntry
{
public:
	CountEntry(SimpleMutex &aMutex, SimpleCondition &aCondition, long &lCount)
		: fMutex(aMutex), fCondition(aCondition), fCount(lCount) {
		StartTrace(CountEntry.CountEntry);
		SimpleMutexEntry sme(fMutex);
		sme.Use();
		++fCount;
		Trace("count:" << fCount);
	}
	~CountEntry() {
		StartTrace(CountEntry.~CountEntry);
		SimpleMutexEntry sme(fMutex);
		sme.Use();
		if (fCount > 0L) {
			--fCount;
		}
		Trace("count:" << fCount);
		Trace("signalling condition");
		fCondition.Signal();
	}
	void Use() const {};

private:
	SimpleMutex	&fMutex;
	SimpleCondition &fCondition;
	long		&fCount;
};

//---- Queue ----------------------------------------------------------------
Queue::Queue(const char *name, long lQueueSize, Allocator *pAlloc)
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
	, fAnyQueue(fAllocator)
	, fAnyStatistics(fAllocator)
{
	StartTrace1(Queue.Queue, "queue size:" << lQueueSize);
	fAnyStatistics["QueueSize"] = lQueueSize;
	Anything anyTime;
	DateTime::GetTimeOfDay(anyTime);
	fAnyStatistics["CreateTime"]["sec"] = anyTime["sec"].AsLong(0L);
	fAnyStatistics["CreateTime"]["usec"] = anyTime["usec"].AsLong(0L);
	UnBlock();
}

Queue::~Queue()
{
	StartTrace(Queue.~Queue);
	// block any pending interface accesses
	Block();

	MutexEntry me(fQueueLock);
	me.Use();
	// mark that we are no longer alive
	fAlive = 0x00dead00;

	long lSize = 0L;
	if ( (lSize = IntGetSize()) > 0L ) {
		SYSWARNING("Destruction of non-empty queue [" << fName << "]! still " << lSize << " Elements in Queue!");
		Anything anyElements;
		IntEmptyQueue(anyElements);
		TraceAny(anyElements, "still in queue");
	}
}

void Queue::IntReleaseBlockedPutters()
{
	StartTrace(Queue.IntReleaseBlockedPutters);
	SimpleMutexEntry sme(fBlockingPutLock);
	sme.Use();
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

void Queue::IntReleaseBlockedGetters()
{
	StartTrace(Queue.IntReleaseBlockedGetters);
	SimpleMutexEntry sme(fBlockingGetLock);
	sme.Use();
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

bool Queue::IsBlocked(BlockingSide aSide)
{
	StartTrace(Queue.IsBlocked);
	if ( IsAlive() ) {
		// need only a read lock here
		RWLockEntry rwe(fBlockedLock, true);
		rwe.Use();
		return ( ( feBlocked & aSide ) == aSide );
	}
	return true;
}

void Queue::UnBlock(BlockingSide aSide)
{
	StartTrace1(Queue.UnBlock, "side:" << aSide);
	if ( IsAlive() ) {
		// need a write lock here
		RWLockEntry rwe(fBlockedLock, false);
		rwe.Use();
		feBlocked = ( BlockingSide )( feBlocked & ( ~aSide & eBothSides ) );
	}
}

void Queue::Block(BlockingSide aSide)
{
	StartTrace1(Queue.Block, "side:" << aSide);
	if ( IsAlive() ) {
		{
			// need a write lock here
			RWLockEntry rwe(fBlockedLock, false);
			rwe.Use();
			feBlocked = ( BlockingSide )( feBlocked | ( aSide & eBothSides ) );
		}
		// release potentially waiting putters/getters
		MutexEntry me(fQueueLock);
		me.Use();
		if ( aSide & ePutSide ) {
			IntReleaseBlockedPutters();
		}
		if ( aSide & eGetSide ) {
			IntReleaseBlockedGetters();
		}
	}
}

Queue::StatusCode Queue::DoPut(Anything &anyElement)
{
	StartTrace(Queue.DoPut);
	StatusCode eRet = eBlocked;
	if ( !IsBlocked(ePutSide) ) {
		MutexEntry me(fQueueLock);
		me.Use();
		long lSize = fAnyQueue.Append(anyElement);
		++fPutCount;
		// need to increment lSize because Append returns index of appended Anything
		++lSize;
		fMaxLoad = itoMAX( fMaxLoad, lSize );
		fSemaFullSlots.Release();
		eRet = eSuccess;
	}
	return eRet;
}

Queue::StatusCode Queue::Put(Anything &anyElement, bool bTryLock)
{
	StartTrace(Queue.Put);
	StatusCode eRet = eBlocked;
	if ( !IsBlocked(ePutSide) ) {
		if ( bTryLock ) {
			eRet = /* eTryAcquireFailed |*/ eFull;
			if ( fSemaEmptySlots.TryAcquire() ) {
				eRet = DoPut(anyElement);
			}
		} else {
			CountEntry ce(fBlockingPutLock, fBlockingPutCond, fBlockingPutCount);
			ce.Use();
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

Queue::StatusCode Queue::DoGet(Anything &anyElement)
{
	StartTrace(Queue.DoGet);
	StatusCode eRet = eBlocked;
	if ( !IsBlocked(eGetSide) ) {
		MutexEntry me(fQueueLock);
		me.Use();
		if ( fAnyQueue.GetSize() ) {
			anyElement = fAnyQueue.At(0L);
			fAnyQueue.Remove(0L);
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

Queue::StatusCode Queue::Get(Anything &anyElement, bool bTryLock)
{
	StartTrace(Queue.Get);
	StatusCode eRet = eBlocked;
	if ( !IsBlocked(eGetSide) ) {
		if ( bTryLock ) {
			eRet = /*eTryAcquireFailed |*/ eEmpty;
			if ( fSemaFullSlots.TryAcquire() ) {
				eRet = DoGet(anyElement);
			}
		} else {
			CountEntry ce(fBlockingGetLock, fBlockingGetCond, fBlockingGetCount);
			ce.Use();
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

long Queue::GetSize()
{
	StartTrace(Queue.GetSize);
	long lSize = -1;
	if ( !IsBlocked() ) {
		MutexEntry me(fQueueLock);
		me.Use();
		lSize = IntGetSize();
	}
	return lSize;
}

void Queue::EmptyQueue(Anything &anyElements)
{
	StartTrace(Queue.EmptyQueue);
	if ( !IsBlocked() ) {
		MutexEntry me(fQueueLock);
		me.Use();
		IntEmptyQueue(anyElements);
	}
}

long Queue::IntGetSize()
{
	StartTrace(Queue.IntGetSize);
	long lRet = fAnyQueue.GetSize();
	Trace("Anything  size:" << lRet);
	Trace("(Put-Get) size:" << (long)(fPutCount - fGetCount));
	return lRet;
}

void Queue::IntEmptyQueue(Anything &anyElements)
{
	StartTrace(Queue.IntEmptyQueue);
	Trace("current queue size:" << fAnyQueue.GetSize());
	Anything anyRef;
	while ( fAnyQueue.GetSize() ) {
		fSemaFullSlots.TryAcquire();
		anyRef = fAnyQueue[0L];
		fAnyQueue.Remove(0L);
		anyElements.Append(anyRef);
		fSemaEmptySlots.Release();
	}
}

void Queue::GetStatistics(Anything &anyStatistics)
{
	StartTrace(Queue.GetStatistics);
	if ( !IsBlocked() ) {
		long lCurSec(0L), lCurUSec(0L);
		{
			MutexEntry me(fQueueLock);
			me.Use();
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
