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

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SysLog.h"
#include "Threads.h"
#include "DataAccess.h"

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
}

void SybCTThreadTest::tearDown ()
{
	StartTrace(SybCTThreadTest.tearDown);
}

class SybTestThread : public Thread
{
	SybCTThreadTest &fMaster;
	long fId;

public:
	SybTestThread(SybCTThreadTest &master, long id)
		: Thread("SybTestThread"), fMaster(master), fId(id)
	{}

protected:
	virtual void Run() {
		fMaster.Run(fId);
	}
};

void SybCTThreadTest::Run(long id)
{
	for (int i = 0; i < 20; i++) {
		SysLog::WriteToStderr(String("running thread #") << id << "\n");

		Context ctx;

		DataAccess da("SybTestThreadDA");
		t_assert(da.StdExec(ctx));

		Anything tmpStore = ctx.GetTmpStore();
		t_assert(tmpStore["SybTestThreadDA"]["QueryResult"].GetSize() > 40);

		DataAccess da2("SybTestThreadWithErrorDA");
		t_assertm( !da2.StdExec(ctx), "expected test to fail because of SQL syntax error");
	}
}

void SybCTThreadTest::test()
{
	StartTrace(SybCTThreadTest.test);

	const long nThreads = 10;
	SybTestThread *threadArray[nThreads];
	long i = 0;
	for (i = 0; i < nThreads; i++) {
		threadArray[i] = new SybTestThread(*this, i);
		threadArray[i]->Start();
	}
	for (i = 0; i < nThreads; i++) {
		threadArray[i]->CheckState(Thread::eTerminated);
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *SybCTThreadTest::suite ()
{
	StartTrace(SybCTThreadTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SybCTThreadTest, test);

	return testSuite;
}
