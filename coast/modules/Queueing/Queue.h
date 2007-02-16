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
#include "IFAObject.h"
#include "Threads.h"
#include <limits.h>

//---- forward declaration -----------------------------------------------

//---- Queue ----------------------------------------------------------
//! <B>Simple, thread-safe, Anything based queue</B>
/*!
Queue elements are represented using Anythings. The internal queue itself is also an Anything which allows
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
class EXPORTDECL_QUEUEING Queue : public IFAObject
{
	friend class QueueTest;
public:
	//--- constructors
	Queue(const char *name, long lQueueSize = LONG_MAX, Allocator *pAlloc = Storage::Global());
	~Queue();

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
	/*! further explanation of the purpose of the method
		this may contain <B>HTML-Tags</B>
		...
		\param aParam explanation of aParam (important : paramname - explanation )
		\return explanation of return value
		\pre explanation of precondition for the method call
		\post explanation of postcondition for the method call
	*/
	StatusCode Put(Anything &anyElement, bool bTryLock = false);
	StatusCode Get(Anything &anyElement, bool bTryLock = false);

	void EmptyQueue(Anything &anyElements);

	long GetSize();
	void GetStatistics(Anything &anyStatistics);

	inline bool IsAlive() {
		return fAlive == 0xf007f007;
	}

	enum BlockingSide {
		eNone = 0,
		ePutSide = 1,
		eGetSide = ePutSide << 1,
		eBothSides = ePutSide | eGetSide,
	};

	bool IsBlocked(BlockingSide aSide = eBothSides);
	void Block(BlockingSide aSide = eBothSides);
	void UnBlock(BlockingSide aSide = eBothSides);

	virtual IFAObject *Clone() const {
		return NULL;
	}

private:
	StatusCode DoPut(Anything &anyElement);
	StatusCode DoGet(Anything &anyElement);
	long IntGetSize();
	void IntEmptyQueue(Anything &anyElements);
	void IntReleaseBlockedPutters();
	void IntReleaseBlockedGetters();

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
	SimpleCondition fBlockingPutCond, fBlockingGetCond;
	Anything 	fAnyQueue, fAnyStatistics;
};

#endif
