/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "WorkerPoolManagerModuleTest.h"

//--- module under test --------------------------------------------------------
#include "WorkerPoolManagerModule.h"
#include "WorkerPoolManagerModulePoolManager.h"
//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "DiffTimer.h"

//--- c-modules used -----------------------------------------------------------

//---- WorkerPoolManagerModuleTest ----------------------------------------------------------------
WorkerPoolManagerModuleTest::WorkerPoolManagerModuleTest(TString tstrName)
	: ConfiguredTestCase(tstrName, "WorkerPoolManagerModuleTestConfig")
{
	StartTrace(WorkerPoolManagerModuleTest.Ctor);
}

WorkerPoolManagerModuleTest::~WorkerPoolManagerModuleTest()
{
	StartTrace(WorkerPoolManagerModuleTest.Dtor);
}

// setup for this ConfiguredTestCase
void WorkerPoolManagerModuleTest::setUp ()
{
	StartTrace(WorkerPoolManagerModuleTest.setUp);
	ConfiguredTestCase::setUp();
}

void WorkerPoolManagerModuleTest::tearDown ()
{
	StartTrace(WorkerPoolManagerModuleTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void WorkerPoolManagerModuleTest::TestReset ()
{
	StartTrace(WorkerPoolManagerModuleTest.TestReset);
	WDModule::Reset(fConfig["ConfigOne"], fConfig["ConfigTwo"]);
}

void WorkerPoolManagerModuleTest::TestWorkerOne()
{
	StartTrace(WorkerPoolManagerModuleTest.TestWorkerOne);

	WDModule::Install(fConfig["ConfigOne"]);
	WorkerPoolManagerModule *pModule = (WorkerPoolManagerModule *)WDModule::FindWDModule("WorkerPoolManagerModule");
	t_assertm(pModule != NULL, "Module should be found");
	FOREACH_ENTRY("TestWorkerOne", cConfig, testName) {
		Check(cConfig, pModule);
	}
}

void WorkerPoolManagerModuleTest::TestWorkerTwo()
{
	StartTrace(WorkerPoolManagerModuleTest.TestWorkerTwo);

	WorkerPoolManagerModule *pModule = (WorkerPoolManagerModule *)WDModule::FindWDModule("WorkerPoolManagerModule");
	t_assertm(pModule != NULL, "Module should be found");
	FOREACH_ENTRY("TestWorkerTwo", cConfig, testName) {
		Check(cConfig, pModule);
	}
	WDModule::Terminate(fConfig["ConfigTwo"]);
}

void WorkerPoolManagerModuleTest::Check(ROAnything cConfig, WorkerPoolManagerModule *pModule)
{
	StartTrace(WorkerPoolManagerModuleTest.Check);
	WorkerPoolManagerModulePoolManager *pPool = pModule->GetPoolManager(cConfig["Pool"].AsString());
	t_assertm(pPool != NULL, "Pool should be found");
	if (pPool) {
		for (long l = 0; l < 5; l++) {
			Anything aMsgAny(cConfig["Message"].DeepClone()), workerConfig, results;
			// create Anything with parameters to pass
			workerConfig["messages"] = (IFAObject *)&aMsgAny;
			workerConfig["results"] = (IFAObject *)&results;
			// this call blocks until the worker has finished working
			{
				Trace("starting worker");
				DiffTimer aTimer;
				pPool->Work(workerConfig);
				Trace("Worker used: " << aTimer.Diff() << "ms");
			}

			// check the result
			assertEquals(results["Got"].AsString(), cConfig["Message"].AsString());
			assertEquals(results["WorkerInitialConfig"].AsString(), cConfig["ExpectedWorkerInitialConfig"].AsString());
			TraceAny(aMsgAny, "Message sent");
			TraceAny(results, "Results");
		}
	}
}

void WorkerPoolManagerModuleTest::test()
{
	StartTrace(WorkerPoolManagerModuleTest.test);
//	t_assertm(false,"test me");
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *WorkerPoolManagerModuleTest::suite ()
{
	StartTrace(WorkerPoolManagerModuleTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, WorkerPoolManagerModuleTest, test);
	ADD_CASE(testSuite, WorkerPoolManagerModuleTest, TestWorkerOne);
	ADD_CASE(testSuite, WorkerPoolManagerModuleTest, TestReset);
	ADD_CASE(testSuite, WorkerPoolManagerModuleTest, TestWorkerTwo);

	return testSuite;
}
