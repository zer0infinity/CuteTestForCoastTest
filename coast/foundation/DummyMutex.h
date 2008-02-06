/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DummyMutex_H
#define _DummyMutex_H

//---- baseclass include -------------------------------------------------

//---- forward declaration -----------------------------------------------
class Allocator;

//---- DummyMutex ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class DummyCondition;

class DummyMutex
{
	friend class DummyCondition;
public:
	typedef DummyCondition ConditionType;
	/*! create mutex with names to ease debugging of locking problems
		\param name a name to identify the mutex when tracing locking problems
		\param a allocator used to allocate the storage for the name */
	DummyMutex(const char *, Allocator *) {}
	~DummyMutex() {}

	/*! tries to acquire the mutex for this thread, blocks the caller if already locked (by another thread)
		acquires the mutex for the calling thread; this is NOT recursive, if you call lock twice for the same thread it deadlocks */
	void Lock() {}

	/*! releases the mutex */
	void Unlock() {}

	/*! tries to acquire the mutex if possible; bails out without locking if already locked
		\return false when already locked */
	bool TryLock() {
		return true;
	}

//	/*! returns native reprentation of mutex hidden by macro MUTEXPTR */
//	MUTEXPTR GetInternal() const {  return GETMUTEXPTR(fMutex); }

private:
	//!standard constructor prohibited
	DummyMutex();
	//!standard copy constructor prohibited
	DummyMutex(const DummyMutex &);
	//!standard assignement operator prohibited
	void operator=(const DummyMutex &);
};

class DummyCondition
{
public:
	typedef DummyMutex MutexType;
	//!creates a system dependent condition variable
	DummyCondition() {}
	//!destroys the system dependent condition variable
	~DummyCondition() {}

	/*! waits for the condition to be signaled atomicly releasing the mutex while waiting and reaquiring it when returning from the wait
		\param m the mutex which is used with the condition; it must be locked, since it will be unlocked
		\return system dependent return value; zero if call was ok */
	int Wait(MutexType &) {
		return 0;
	}

	/*! waits for the condition to be signaled at most secs seconds and nanosecs nanoseconds if available on the plattform. After this time we block on the SimpleMutex until we can lock it before leaving the function
		\param m the mutex which is used with the condition; it must be locked, since it will be unlocked
		\param secs timeout to wait in seconds
		\param nanosecs timeout to wait in nanoseconds (10e-9)
		\return system dependent return value; zero if call was ok */
	int TimedWait(MutexType &, long, long) {
		return 0;
	}

	//!access the system dependent handle to the condition variable
//	CONDPTR GetInternal() { return GETCONDPTR(fSimpleCondition); }
	//!signal the condition; do it while protected through the associated mutex, then unlock the mutex; zero or one waiting thread will be woken up
	int Signal() {
		return 0;
	}
	//!broadcasts the condition; do it while protected through the associated mutex, then unlock the mutex; all waiting thread will eventually be woken up
	int BroadCast() {
		return 0;
	}

private:
	//!get internal 'Id' mainly for Tracing
	long GetId() {
		return -1;
	}
};

#endif
