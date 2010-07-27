/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DummyMutex_H
#define _DummyMutex_H

//---- forward declaration -----------------------------------------------
class Allocator;
class DummyCondition;

//---- DummyMutex ----------------------------------------------------------
//! <B>Dummy implementation of Mutex</B>
/*! Can be used to instantiate classes using Mutex interface without any locking effects.
*/
class DummyMutex
{
	friend class DummyCondition;
public:
	typedef DummyCondition ConditionType;
	/*! create mutex with names to ease debugging of locking problems
		\param name a name to identify the mutex when tracing locking problems
		\param a allocator used to allocate the storage for the name */
	DummyMutex(const char *name, Allocator *a) {}
	~DummyMutex() {}

	/*! Do nothing lock call */
	void Lock() {}

	/*! Do nothing unlock call */
	void Unlock() {}

	/*! Do nothing trylock call
		\return true always */
	bool TryLock() {
		return true;
	}

private:
	//!standard constructor prohibited
	DummyMutex();
	//!standard copy constructor prohibited
	DummyMutex(const DummyMutex &);
	//!standard assignement operator prohibited
	void operator=(const DummyMutex &);
};

//! <B>Dummy implementation of Condition</B>
/*! Can be used to instantiate classes using Condition interface without any blocking effects.
*/
class DummyCondition
{
public:
	typedef DummyMutex MutexType;
	//!creates a system dependent condition variable
	DummyCondition() {}
	//!destroys the system dependent condition variable
	~DummyCondition() {}

	/*! Do nothing wait call
		\return 0 always */
	int Wait(MutexType &) {
		return 0;
	}

	/*! Do nothing timed wait call
		\return 0 always */
	int TimedWait(MutexType &m, long secs, long nanosec) {
		return 0;
	}

	/*! Do nothing signal call
	 * @return 0 always */
	int Signal() {
		return 0;
	}
	/*! Do nothing broadcast call
	 * @return 0 always */
	int BroadCast() {
		return 0;
	}

private:
	/*! Do nothing fake id call
	 * @return -1 always */
	long GetId() {
		return -1;
	}
};

#endif
