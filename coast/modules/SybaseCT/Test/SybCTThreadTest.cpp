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
#include "SysLog.h"
#include "Threads.h"
#include "DataAccess.h"
#include "DiffTimer.h"
#include "Timers.h"
#include "System.h"

//--- c-modules used -----------------------------------------------------------

//---- SybCTThreadTest ----------------------------------------------------------------
SybCTThreadTest::SybCTThreadTest(TString name)
	: ConfiguredTestCase(name, "Config")
{
	StartTrace(SybCTThreadTest.Ctor);
}

SybCTThreadTest::~SybCTThreadTest()
{
	StartTrace(SybCTThreadTest.Dtor);
}

// setup for this ConfiguredTestCase
void SybCTThreadTest::setUp ()
{
	StartTrace(SybCTThreadTest.setUp);
	ConfiguredTestCase::setUp();
	fbWasInitialized = SybCTnewDAImpl::fgInitialized;
	if ( fbWasInitialized ) {
		SybCTnewDAImpl::Finis();
	}
}

void SybCTThreadTest::tearDown ()
{
	StartTrace(SybCTThreadTest.tearDown);
	// set initialized state back here
	if ( fbWasInitialized ) {
		Anything anyConfig;
		if ( t_assert(System::LoadConfigFile(anyConfig, "Config")) ) {
			t_assert(SybCTnewDAImpl::Init(anyConfig));
		}
	}
	ConfiguredTestCase::tearDown();
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
		Context ctx(fConfig);

		DataAccess da(goodDAName);
		t_assert(da.StdExec(ctx));
		RequestTimeLogger(ctx);

		Anything tmpStore = ctx.GetTmpStore();
		t_assertm(tmpStore[goodDAName]["QueryResult"].GetSize() > 40, goodDAName);

		DataAccess da2(failDAName);
		t_assertm( !da2.StdExec(ctx), TString(failDAName) << " expected test to fail because of SQL syntax error");
		RequestTimeLogger(ctx);
	}
}

void SybCTThreadTest::SybCTDAImplTest()
{
	StartTrace(SybCTThreadTest.SybCTDAImplTest);
	DiffTimer aTimer;
	DoTest("SybTestThreadDA", "SybTestThreadDAWithError");
	SysLog::WriteToStderr(String("elapsed time for SybCTDAImplTest:") << (long)aTimer.Diff() << "ms\n");
}

void SybCTThreadTest::SybCTPoolDAImplTest()
{
	StartTrace(SybCTThreadTest.SybCTPoolDAImplTest);
	DiffTimer aTimer;
	DoTest("SybTestThreadPoolDA", "SybTestThreadPoolDAWithError");
	SysLog::WriteToStderr(String("elapsed time for SybCTPoolDAImplTest:") << (long)aTimer.Diff() << "ms\n");
}

void SybCTThreadTest::SybCTnewDAImplTest()
{
	StartTrace(SybCTThreadTest.SybCTnewDAImplTest);
	TraceAny(fTestCaseConfig, "case config");
	if ( t_assert(SybCTnewDAImpl::Init(fTestCaseConfig)) ) {
		DiffTimer aTimer;
		DoTest("SybTestThreadnewDA", "SybTestThreadnewDAWithError");
		SysLog::WriteToStderr(String("elapsed time for SybCTnewDAImplTest:") << (long)aTimer.Diff() << "ms\n");
	}
}

void SybCTThreadTest::DoTest(const char *goodDAName, const char *failDAName)
{
	StartTrace(SybCTThreadTest.DoTest);

	const long nThreads = 10;
	const long lPoolSize = 1000;
	const long lPoolBuckets = 20L;
	SybTestThread *threadArray[nThreads];
	long i = 0;
	for (i = 0; i < nThreads; i++) {
		threadArray[i] = new SybTestThread(*this, i, goodDAName, failDAName);
		threadArray[i]->Start(MT_Storage::MakePoolAllocator(lPoolSize, lPoolBuckets, i));
	}
	for (i = 0; i < nThreads; i++) {
		threadArray[i]->CheckState(Thread::eTerminated);
	}
	for (i = 0; i < nThreads; i++) {
		delete threadArray[i];
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *SybCTThreadTest::suite ()
{
	StartTrace(SybCTThreadTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SybCTThreadTest, SybCTDAImplTest);
	ADD_CASE(testSuite, SybCTThreadTest, SybCTPoolDAImplTest);
	ADD_CASE(testSuite, SybCTThreadTest, SybCTnewDAImplTest);

	return testSuite;
}
