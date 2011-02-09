/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTThreadTest.h"

//--- module under test --------------------------------------------------------
#include "SybCTnewDAImpl.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SystemLog.h"
#include "Threads.h"
#include "DataAccess.h"
#include "DiffTimer.h"
#include "Timers.h"

//--- c-modules used -----------------------------------------------------------

//---- SybCTThreadTest ----------------------------------------------------------------
SybCTThreadTest::SybCTThreadTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(SybCTThreadTest.SybCTThreadTest);
}

SybCTThreadTest::~SybCTThreadTest()
{
	StartTrace(SybCTThreadTest.Dtor);
}

void SybCTThreadTest::setUp ()
{
	StartTrace(SybCTThreadTest.setUp);
	fbWasInitialized = SybCTnewDAImpl::fgInitialized;
	if ( fbWasInitialized ) {
		SybCTnewDAImpl::Finis();
	}
}

void SybCTThreadTest::tearDown ()
{
	StartTrace(SybCTThreadTest.tearDown);
}

class SybTestThread : public Thread
{
	SybCTThreadTest &fMaster;
	long fId;
	const char *fGoodDAName, *fFailDAName;

public:
	SybTestThread(SybCTThreadTest &master, long id, const char *goodDAName, const char *failDAName)
		: Thread("SybTestThread"), fMaster(master), fId(id), fGoodDAName(goodDAName), fFailDAName(failDAName)
	{}

protected:
	virtual void Run() {
		fMaster.Run(fId, fGoodDAName, fFailDAName);
	}
};

void SybCTThreadTest::Run(long id, const char *goodDAName, const char *failDAName)
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

void SybCTThreadTest::SybCTnewDAImplTest()
{
	StartTrace(SybCTThreadTest.SybCTnewDAImplTest);
	TraceAny(GetTestCaseConfig(), "case config");
	if ( t_assert(SybCTnewDAImpl::Init(GetTestCaseConfig())) ) {
		DiffTimer aTimer;
		DoTest("SybTestThreadnewDA", "SybTestThreadnewDAWithError");
		SystemLog::WriteToStderr(String("elapsed time for SybCTnewDAImplTest:") << (long)aTimer.Diff() << "ms\n");
	}
}

void SybCTThreadTest::DoTest(const char *goodDAName, const char *failDAName)
{
	StartTrace(SybCTThreadTest.DoTest);

	long nThreads = GetTestCaseConfig()["ThreadPoolSize"].AsLong(10L);
	u_long lPoolSize = (u_long)GetTestCaseConfig()["PoolStorageSize"].AsLong(1000L);
	u_long lPoolBuckets = (u_long)GetTestCaseConfig()["NumOfPoolBucketSizes"].AsLong(20L);

	SybTestThread **threadArray = new SybTestThread*[nThreads];
	long i = 0;
	for (i = 0; i < nThreads; i++) {
		threadArray[i] = new (Coast::Storage::Global()) SybTestThread(*this, i, goodDAName, failDAName);
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
Test *SybCTThreadTest::suite ()
{
	StartTrace(SybCTThreadTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SybCTThreadTest, SybCTnewDAImplTest);
	return testSuite;
}
