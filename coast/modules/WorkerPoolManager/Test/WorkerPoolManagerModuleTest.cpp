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
#include "DiffTimer.h"

//--- c-modules used -----------------------------------------------------------

//---- WorkerPoolManagerModuleTest ----------------------------------------------------------------
WorkerPoolManagerModuleTest::WorkerPoolManagerModuleTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(WorkerPoolManagerModuleTest.WorkerPoolManagerModuleTest);
}

TString WorkerPoolManagerModuleTest::getConfigFileName()
{
	return "WorkerPoolManagerModuleTestConfig";
}

WorkerPoolManagerModuleTest::~WorkerPoolManagerModuleTest()
{
	StartTrace(WorkerPoolManagerModuleTest.Dtor);
}

void WorkerPoolManagerModuleTest::TestReset ()
{
	StartTrace(WorkerPoolManagerModuleTest.TestReset);
	WDModule::Reset(GetConfig()["ConfigOne"], GetConfig()["ConfigTwo"]);
}

void WorkerPoolManagerModuleTest::TestWorkerOne()
{
	StartTrace(WorkerPoolManagerModuleTest.TestWorkerOne);

	WDModule::Install(GetConfig()["ConfigOne"]);
	WorkerPoolManagerModule *pModule = (WorkerPoolManagerModule *)WDModule::FindWDModule("WorkerPoolManagerModule");
	t_assertm(pModule != NULL, "Module should be found");
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		Check(cConfig, pModule);
	}
}

void WorkerPoolManagerModuleTest::TestWorkerTwo()
{
	StartTrace(WorkerPoolManagerModuleTest.TestWorkerTwo);

	WorkerPoolManagerModule *pModule = (WorkerPoolManagerModule *)WDModule::FindWDModule("WorkerPoolManagerModule");
	t_assertm(pModule != NULL, "Module should be found");
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		Check(cConfig, pModule);
	}
	WDModule::Terminate(GetConfig()["ConfigTwo"]);
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

// builds up a suite of tests, add a line for each testmethod
Test *WorkerPoolManagerModuleTest::suite ()
{
	StartTrace(WorkerPoolManagerModuleTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, WorkerPoolManagerModuleTest, TestWorkerOne);
	ADD_CASE(testSuite, WorkerPoolManagerModuleTest, TestReset);
	ADD_CASE(testSuite, WorkerPoolManagerModuleTest, TestWorkerTwo);
	return testSuite;
}
