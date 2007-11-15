/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ThreadPoolTest.h"

//--- module under test --------------------------------------------------------
#include "TestThread.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- ThreadPoolTest ----------------------------------------------------------------
ThreadPoolTest::ThreadPoolTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(ThreadPoolTest.Ctor);
}

ThreadPoolTest::~ThreadPoolTest()
{
	StartTrace(ThreadPoolTest.Dtor);
}

void ThreadPoolTest::JoinTest()
{
	StartTrace(ThreadPoolTest.JoinTest);

	TestThreadPool ttp;

	Anything dummy;
	if ( t_assertm(ttp.Init(10, dummy), "Init failed") ) {
		assertEqualm(0L, ttp.Start(false, 0, 0), "Start failed");
		t_assert(ttp.AllThreadsStarted());

		Trace("Joining Threads...");
		ttp.Join();
		t_assert(ttp.AllThreadsTerminated());
		assertEqualm(0L, ttp.Terminate(1, 10), "expected all threads to be terminated");
	}
	// restart pool to see if fStartetThreads is set to the correct value
	if ( t_assertm(ttp.Init(5, dummy), "Init failed") ) {
		assertEqualm(0L, ttp.Start(false, 0, 0), "Start failed");
		t_assert(ttp.AllThreadsStarted());
		assertEqualm(0L, ttp.Terminate(1, 10), "expected all threads to be terminated");
		t_assert(ttp.AllThreadsTerminated());
	}
}

void ThreadPoolTest::TerminateTest()
{
	StartTrace(ThreadPoolTest.TerminateTest);

	TerminateTestThreadPool ttp;

	Anything dummy;
	if ( t_assertm(ttp.Init(10, dummy), "Init failed") ) {
		assertEqualm(0L, ttp.Start(false, 0, 0), "Start failed");
		t_assert(ttp.AllThreadsStarted());
		assertEqualm(0L, ttp.Terminate(1, 10), "expected all threads to be terminated");
		t_assert(ttp.AllThreadsTerminated());
	}
	// restart pool to see if fStartetThreads is set to the correct value
	if ( t_assertm(ttp.Init(5, dummy), "Init failed") ) {
		assertEqualm(0L, ttp.Start(false, 0, 0), "Start failed");
		t_assert(ttp.AllThreadsStarted());
		assertEqualm(0L, ttp.Terminate(1, 10), "expected all threads to be terminated");
		t_assert(ttp.AllThreadsTerminated());
	}
}

template
<
typename TArgs
>
class GugusThread : public Observable<GugusThread<TArgs>, TArgs>
{
	//!prohibit the use of the copy constructor
	GugusThread(const GugusThread &);
	//!prohibit the use of the assignement operator
	GugusThread &operator=(const GugusThread &);
public:
	typedef Observable<GugusThread, TArgs> tBaseClass;

	/*! does nothing, real work is done in init, object is still unusable without call of Init() method!
		\param name Give the pool Manager a name, gets printed in statistics */
	GugusThread(const char *name, Allocator *a)
		: tBaseClass(name, a)
	{}

	/*! prints some statistics and terminates
		\note can't delete pool here because representation of subobject has already gone */
	virtual ~GugusThread() {};
};

template
<
typename TObservedType,
		 typename TArgs
		 >
class TestPoolManager : public PoolManager<TObservedType, TArgs>
{
public:
	typedef PoolManager<TObservedType, TArgs> tBaseClass;
	typedef typename tBaseClass::tObservedPtr tObservedPtr;
	typedef typename tBaseClass::tArgsRef tArgsRef;

	/*! does nothing, real work is done in init, object is still unusable without call of Init() method!
		\param name Give the pool Manager a name, gets printed in statistics */
	TestPoolManager(const char *name)
		: tBaseClass(name) {};

	/*! prints some statistics and terminates
		\note can't delete pool here because representation of subobject has already gone */
	virtual ~TestPoolManager() {};

	//! implements the StatGatherer interface used by StatObserver
	void DoGetStatistic(Anything &item) { }

	void Update(tObservedPtr pObserved, tArgsRef aUpdateArgs) {
		fanyUpdates.Append(aUpdateArgs.DeepClone());
	}

	Anything fanyUpdates;
};

void ThreadPoolTest::PoolManagerTest()
{
	StartTrace(ThreadPoolTest.PoolManagerTest);
	TestPoolManager<GugusThread<ROAnything>, ROAnything> myPool("MyTestPool");
	GugusThread<ROAnything> myThread("MyGugus", Storage::Global());
	myThread.AddObserver(&myPool);
	Anything anyData, anyExpected;
	anyData["Mytest"] = 33;
	myThread.NotifyAll(anyData);
	anyExpected.Append(anyData);
	assertAnyEqual(anyExpected, myPool.fanyUpdates);
}

// builds up a suite of testcases, add a line for each testmethod
Test *ThreadPoolTest::suite ()
{
	StartTrace(ThreadPoolTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ThreadPoolTest, JoinTest);
	ADD_CASE(testSuite, ThreadPoolTest, TerminateTest);
	ADD_CASE(testSuite, ThreadPoolTest, PoolManagerTest);

	return testSuite;
}
