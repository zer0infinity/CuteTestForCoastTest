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
	: TestCase(tname)
{
	StartTrace(ThreadPoolTest.Ctor);
}

ThreadPoolTest::~ThreadPoolTest()
{
	StartTrace(ThreadPoolTest.Dtor);
}

// setup for this TestCase
void ThreadPoolTest::setUp ()
{
	StartTrace(ThreadPoolTest.setUp);
} // setUp

void ThreadPoolTest::tearDown ()
{
	StartTrace(ThreadPoolTest.tearDown);
} // tearDown

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
	StartTrace(ThreadPoolTest.JoinTest);

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

// builds up a suite of testcases, add a line for each testmethod
Test *ThreadPoolTest::suite ()
{
	StartTrace(ThreadPoolTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(ThreadPoolTest, JoinTest));
	testSuite->addTest (NEW_CASE(ThreadPoolTest, TerminateTest));

	return testSuite;
} // suite
