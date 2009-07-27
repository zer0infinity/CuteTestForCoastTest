/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MultiThreadedTest.h"

//--- module under test --------------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SysLog.h"
#include "Threads.h"
#include "DataAccess.h"
#include "DiffTimer.h"
#include "Timers.h"
#include "System.h"

//--- c-modules used -----------------------------------------------------------

//---- MultiThreadedTest ----------------------------------------------------------------
MultiThreadedTest::MultiThreadedTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(MultiThreadedTest.MultiThreadedTest);
}

MultiThreadedTest::~MultiThreadedTest()
{
	StartTrace(MultiThreadedTest.Dtor);
}

void MultiThreadedTest::setUp ()
{
	StartTrace(MultiThreadedTest.setUp);
}

void MultiThreadedTest::tearDown ()
{
	StartTrace(MultiThreadedTest.tearDown);
}

class DATestThread : public Thread
{
	MultiThreadedTest &fMaster;
	long fId;
	const char *fGoodDAName, *fFailDAName;

public:
	DATestThread(MultiThreadedTest &master, long id, const char *goodDAName, const char *failDAName)
		: Thread("DATestThread"), fMaster(master), fId(id), fGoodDAName(goodDAName), fFailDAName(failDAName)
	{}

protected:
	virtual void Run() {
		fMaster.Run(fId, fGoodDAName, fFailDAName);
	}
};

void MultiThreadedTest::Run(long id, const char *goodDAName, const char *failDAName)
{
	for (int i = 0; i < 10; i++) {
		Anything aEnv = GetConfig().DeepClone();
		Context ctx(aEnv);

		DataAccess da(goodDAName);
		t_assert(da.StdExec(ctx));
		RequestTimeLogger(ctx);

		Anything tmpStore = ctx.GetTmpStore();
		assertComparem(30L, less_equal, tmpStore[goodDAName]["QueryResult"].GetSize(), goodDAName);

		DataAccess da2(failDAName);
		t_assertm( !da2.StdExec(ctx), TString(failDAName) << " expected test to fail because of SQL syntax error");
		RequestTimeLogger(ctx);
	}
}

void MultiThreadedTest::DAImplTest()
{
	StartTrace(MultiThreadedTest.DAImplTest);
	TraceAny(GetTestCaseConfig(), "case config");
	DiffTimer aTimer;
	DoTest(GetTestCaseConfig()["SuccessDA"].AsString("MultiThreadedDA"), GetTestCaseConfig()["FailureDA"].AsString("MultiThreadedDAWithError"));
	SysLog::WriteToStderr(String("elapsed time for DAImplTest:") << (long)aTimer.Diff() << "ms\n");
}

void MultiThreadedTest::DoTest(const char *goodDAName, const char *failDAName)
{
	StartTrace(MultiThreadedTest.DoTest);

	long nThreads = GetTestCaseConfig()["ThreadPoolSize"].AsLong(10L);
	u_long lPoolSize = (u_long)GetTestCaseConfig()["PoolStorageSize"].AsLong(1000L);
	u_long lPoolBuckets = (u_long)GetTestCaseConfig()["NumOfPoolBucketSizes"].AsLong(20L);

	DATestThread **threadArray = new DATestThread*[nThreads];
	long i = 0;
	for (i = 0; i < nThreads; i++) {
		threadArray[i] = new DATestThread(*this, i, goodDAName, failDAName);
		threadArray[i]->Start(MT_Storage::MakePoolAllocator(lPoolSize, lPoolBuckets, i));
	}
	for (i = 0; i < nThreads; i++) {
		threadArray[i]->CheckState(Thread::eTerminated);
	}
	for (i = 0; i < nThreads; i++) {
		delete threadArray[i];
	}
	delete[] threadArray;
}

// builds up a suite of tests, add a line for each testmethod
Test *MultiThreadedTest::suite ()
{
	StartTrace(MultiThreadedTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MultiThreadedTest, DAImplTest);
	return testSuite;
}
