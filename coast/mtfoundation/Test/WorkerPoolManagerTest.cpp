/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Threads.h"
#include "TestThread.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ThreadPools.h"

//--- interface include --------------------------------------------------------
#include "WorkerPoolManagerTest.h"

//---- WorkerPoolManagerTest ----------------------------------------------------------------
WorkerPoolManagerTest::WorkerPoolManagerTest(TString tname) : TestCase(tname), fCheckMutex("WorkerPoolManager")
{
	StartTrace(WorkerPoolManagerTest.Ctor);
}

WorkerPoolManagerTest::~WorkerPoolManagerTest()
{
	StartTrace(WorkerPoolManagerTest.Dtor);
}

// builds up a suite of testcases, add a line for each testmethod
Test *WorkerPoolManagerTest::suite ()
{
	StartTrace(WorkerPoolManagerTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, WorkerPoolManagerTest, InitTest);
	ADD_CASE(testSuite, WorkerPoolManagerTest, EnterLeaveTests);

	return testSuite;
}

// setup for this TestCase
void WorkerPoolManagerTest::setUp ()
{
	StartTrace(WorkerPoolManagerTest.setUp);
} // setUp

void WorkerPoolManagerTest::tearDown ()
{
	StartTrace(WorkerPoolManagerTest.tearDown);
} // tearDown

void WorkerPoolManagerTest::InitTest()
{
	StartTrace(WorkerPoolManagerTest.InitTest);
	SamplePoolManager wpm("InitTestPool");

	Anything config;
	config["timeout"] = 1;
	config["test"] = Anything((IFAObject *)this);
	t_assert(wpm.Init(0, 0, 0, 0, ROAnything(config)) == 0);	// always allocate at least one worker
	t_assert(wpm.MaxRequests2Run() == 1);
	t_assert(wpm.NumOfRequestsRunning() == 0);
	t_assert(wpm.Init(-1, 0, 0, 0, ROAnything(config)) == 0);
	t_assert(wpm.MaxRequests2Run() == 1);
	t_assert(wpm.NumOfRequestsRunning() == 0);
	t_assert(wpm.Init(1, 0, 0, 0, ROAnything(config)) == 0);
	t_assert(wpm.MaxRequests2Run() == 1);
	t_assert(wpm.NumOfRequestsRunning() == 0);
	t_assert(wpm.Init(10, 0, 0, 0, ROAnything(config)) == 0);
	t_assert(wpm.MaxRequests2Run() == 10);
	t_assert(wpm.NumOfRequestsRunning() == 0);

	for (long i = 0; i < 10; i++) {
		t_assertm(wpm.DoGetWorker(i) != 0, "expected 10 Workers to be there");
	}
}

void WorkerPoolManagerTest::EnterLeaveTests()
{
	StartTrace(WorkerPoolManagerTest.EnterLeaveTests);
	const long cPoolSz = 2;
	SamplePoolManager wpm("EnterLeaveTestsPool");
	Anything config;
	config["timeout"] = 1;
	config["test"] = Anything((IFAObject *)this);
	t_assert(wpm.Init(cPoolSz, 0, 0, 0, ROAnything(config)) == 0);
	t_assert(wpm.MaxRequests2Run() == cPoolSz);
	t_assert(wpm.NumOfRequestsRunning() == 0);

	Anything work;
	for (long i = 0; i < 2 * cPoolSz; i++) {
		wpm.Enter(ROAnything(work));
	}
	t_assert(wpm.AwaitEmpty(5));
	t_assert(wpm.Terminate());

	Anything expected;
	Anything statistic;
	wpm.Statistic(statistic);
	expected["PoolSize"] = 2;
	expected["CurrentParallelRequests"] = 0;
	expected["MaxParallelRequests"] = 2;
	expected["TotalRequests"] = 4;
	statistic.Remove("TotalTime"); // since varies
	statistic.Remove("AverageTime"); // since varies
	statistic.Remove("TRX/sec"); // since varies

	assertAnyEqualm(expected, statistic, "statistic differs");
}

void WorkerPoolManagerTest::CheckPrepare2Run(bool isReady, bool wasPrepared)
{
	MutexEntry me(fCheckMutex);
	me.Use();
	t_assertm(isReady, "Worker must be in ready state");
	t_assertm(!wasPrepared, "Worker must not be prepared");
}

void WorkerPoolManagerTest::CheckProcessWorkload(bool isWorking, bool wasPrepared)
{
	MutexEntry me(fCheckMutex);
	me.Use();
	t_assertm(isWorking, "Worker must be in working state");
	t_assertm(wasPrepared, "Worker must have been prepared");
}
