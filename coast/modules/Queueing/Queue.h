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
class Queue : public IFAObject
{
public:
	//--- constructors
	Queue(const char *name, long lQueueSize = LONG_MAX, Allocator *pAlloc = Storage::Global());
	~Queue();

	/*! further explanation of the purpose of the method
		this may contain <B>HTML-Tags</B>
		...
		\param aParam explanation of aParam (important : paramname - explanation )
		\return explanation of return value
		\pre explanation of precondition for the method call
		\post explanation of postcondition for the method call
	*/
	bool Put(Anything &anyElement, bool bTryLock = false);
	bool Get(Anything &anyElement, bool bTryLock = false);

	void EmptyQueue(Anything &anyElements);

	long GetSize();
	void GetStatistics(Anything &anyStatistics);

	inline bool IsAlive() {
		return fAlive == 0xf007f007;
	}
	bool IsBlocked();
	void Block();
	void UnBlock();

	virtual IFAObject *Clone() const {
		return NULL;
	}

private:
	bool DoPut(Anything &anyElement);
	bool DoGet(Anything &anyElement);
	long IntGetSize();
	void IntEmptyQueue(Anything &anyElements);

	String		fName;
	Allocator	*fAllocator;
	Semaphore	fSemaFullSlots, fSemaEmptySlots;
	ul_long		fPutCount, fGetCount;
	long		fMaxLoad, fBlockingPutCount, fBlockingGetCount;
	u_long		fAlive;
	bool		fBlocked;
	Mutex		fQueueLock;
	RWLock		fBlockedLock;
	SimpleMutex fBlockingPutLock, fBlockingGetLock;
	SimpleCondition fBlockingPutCond, fBlockingGetCond;
	Anything 	fAnyQueue, fAnyStatistics;
};

#endif
