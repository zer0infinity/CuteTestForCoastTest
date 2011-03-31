/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "Threads.h"
#include "TestThread.h"
#include "Dbg.h"
#include "TestSuite.h"
#include "ThreadPools.h"
#include "WorkerPoolManagerTest.h"

//---- WorkerPoolManagerTest ----------------------------------------------------------------
WorkerPoolManagerTest::WorkerPoolManagerTest(TString tname) : TestCaseType(tname), fCheckMutex("WorkerPoolManager")
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

void WorkerPoolManagerTest::InitTest()
{
	StartTrace(WorkerPoolManagerTest.InitTest);
	SamplePoolManager wpm("InitTestPool");

	Anything config;
	config["timeout"] = 1;
	config["test"] = Anything((IFAObject *)this);
	t_assert(wpm.Init(0, 0, 0, 0, ROAnything(config)) == 0);	// always allocate at least one worker
	t_assert(wpm.GetPoolSize() == 1);
	t_assert(wpm.ResourcesUsed() == 0);
	t_assert(wpm.Init(-1, 0, 0, 0, ROAnything(config)) == 0);
	t_assert(wpm.GetPoolSize() == 1);
	t_assert(wpm.ResourcesUsed() == 0);
	t_assert(wpm.Init(1, 0, 0, 0, ROAnything(config)) == 0);
	t_assert(wpm.GetPoolSize() == 1);
	t_assert(wpm.ResourcesUsed() == 0);
	t_assert(wpm.Init(10, 0, 0, 0, ROAnything(config)) == 0);
	t_assert(wpm.GetPoolSize() == 10);
	t_assert(wpm.ResourcesUsed() == 0);

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
	t_assert(wpm.GetPoolSize() == cPoolSz);
	t_assert(wpm.ResourcesUsed() == 0);

	Anything work;
	for (long i = 0; i < 2 * cPoolSz; i++) {
		wpm.Enter( ROAnything(work), -1L);
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
	statistic.Remove("TotalTime [ms]"); // since varies
	statistic.Remove("AverageTime [ms]"); // since varies
	statistic.Remove("TRX/sec"); // since varies

	assertAnyEqualm(expected, statistic, "statistic differs");
}

void WorkerPoolManagerTest::CheckProcessWorkload(bool isWorking, bool wasPrepared)
{
	LockUnlockEntry me(fCheckMutex);
	t_assertm(isWorking, "Worker must be in working state");
	t_assertm(wasPrepared, "Worker must have been prepared");
}
