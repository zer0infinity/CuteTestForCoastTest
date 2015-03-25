/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ThreadsTest_H
#define _ThreadsTest_H

#include "TestCase.h"
#include "Threads.h"
#include "DiffTimer.h"

class ThreadsTest: public testframework::TestCase {
public:
	ThreadsTest(TString tname) :
		TestCaseType(tname), fMutex(tname) {
	}
	//!builds up a suite of testcases for this test
	static Test *suite();

	//!tests for argument passing in hooks
	void ThreadHookArgsTest();
	//!tests for thread reuse
	void ThreadObjectReuseTest();
	//!tests for thread state handling
	void ThreadStateTransitionTest();
	//!tests for thread running state handling
	void ThreadRunningStateTransitionTest();
	//!locks a mutex a thousand times
	void MutexLockIterationTest();
	//!checks if the waited time is at least the expected amount
	void TimedRunTest();
	void CheckTime(long time);
	void SimpleLockedAccess(long i);

	//!test mutex behaviour
	void SimpleMutexTest();
	//!test Semaphore behaviour
	void SimpleSemaphoreTest();
	//!test Semaphore behaviour with multiple resources
	void MultiSemaphoreTest();
    //!test recursive mutex behaviour
	void RecursiveMutexTest();
	//!simple nested mutexes in one thread
	void SimpleRecursiveTest();
	//!simple nested mutexes in one thread
	void SimpleRecursiveTryLockTest();
	//!two thread nested mutexes test with signalling
	void TwoThreadRecursiveTest();
	//!two thread nested mutexes test with signalling
	void TwoThreadRecursiveTryLockTest();

protected:
	//--- subclass api
	void LockedIteration(long iterations);

	Anything fLockedAny;
	Mutex fMutex;
	DiffTimer fDiffTimer;
	Condition fCond;
};

#endif
