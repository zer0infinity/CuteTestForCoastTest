/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ThreadedTimeStampTest.h"

//--- module under test --------------------------------------------------------
#include "TimeStamp.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "TimeStampTestThread.h"
#include "StringStream.h"
#include "AnyUtils.h"
#include "SysLog.h"
#include "System.h"
#include "Dbg.h"

//---- ThreadedTimeStampTest ----------------------------------------------------------------
ThreadedTimeStampTest::ThreadedTimeStampTest(TString name)
	: ConfiguredTestCase(name, "ThreadedTimeStampTestConfig"), fCheckMutex("ThreadedTimeStampTest")
{
	StartTrace(ThreadedTimeStampTest.Ctor);
}

ThreadedTimeStampTest::~ThreadedTimeStampTest()
{
	StartTrace(ThreadedTimeStampTest.Dtor);
}

// setup for this TestCase
void ThreadedTimeStampTest::setUp ()
{
	StartTrace(ThreadedTimeStampTest.setUp);
	ConfiguredTestCase::setUp();
}

void ThreadedTimeStampTest::tearDown ()
{
	StartTrace(ThreadedTimeStampTest.tearDown);
	ConfiguredTestCase::tearDown();
}

// builds up a suite of testcases, add a line for each testmethod
Test *ThreadedTimeStampTest::suite ()
{
	StartTrace(ThreadedTimeStampTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(ThreadedTimeStampTest, TimestampConcurrencyTest));
	return testSuite;

} // suite

void ThreadedTimeStampTest::TimestampConcurrencyTest()
{
	StartTrace(ThreadedTimeStampTest.TimestampConcurrencyTest);
	iostream *pIos = System::OpenOStream("ThreadedTimeStampTest", "txt", ios::app);
	(*pIos) << setw(95) << setfill('-') << "---\n" << flush;
	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		ROAnything cConfig = fTestCaseConfig[i];
		Trace("At testindex: " << i);
		long numberOfRuns(cConfig["NumberOfRuns"].AsLong());
		long numberOfThreads(cConfig["NumberOfThreads"].AsLong());
		long concurrencyFactor(cConfig["ConcurrencyFactor"].AsLong());
		DoTimeStampConcurrencyTest(pIos, numberOfRuns, numberOfThreads, concurrencyFactor, cConfig);
	}
	delete pIos;
}

void ThreadedTimeStampTest::DoTimeStampConcurrencyTest(iostream *pIos, long numberOfRuns, long numberOfThreads, long concurrencyFactor, ROAnything roaConfig)
{
	StartTrace(ThreadedTimeStampTest.DoTimeStampConcurrencyTest);
	SamplePoolManager wpm("InitTestPool");
	Anything config;
	config["NumberOfRuns"] = numberOfRuns;
	config["timeout"] = 0;
	config["CompareStamps"] = roaConfig["CompareStamps"].AsBool(false);
	config["UTCCtorTest"] = roaConfig["UTCCtorTest"].AsBool(false);
	config["test"] = Anything((IFAObject *)this);
	t_assert(wpm.Init(numberOfThreads, 1, 10, 20, config) == 0);
	t_assert(wpm.MaxRequests2Run() == numberOfThreads);
	t_assert(wpm.NumOfRequestsRunning() == 0);
	String strRemainder;
	strRemainder << "Threads: " << (numberOfThreads > 9 ? "" : " ") << numberOfThreads << " Concurrency: " << concurrencyFactor << " Runs: " << numberOfRuns << " UTC-Test: " << (roaConfig["UTCCtorTest"].AsBool(false) ? "true " : "false") << " Compare: " << (roaConfig["CompareStamps"].AsBool(false) ? "true " : "false") << "\n";
	Trace(TimeStamp().AsString() << " Start    " << strRemainder);
	SysLog::WriteToStderr(strRemainder);
	for (long i = 0; i < numberOfThreads * concurrencyFactor; i++) {
		wpm.Enter(config);
	}
	t_assert(wpm.AwaitEmpty(1000));
	Trace(TimeStamp().AsString() << " End      " << strRemainder);
	t_assert(wpm.Terminate());

	Anything expected;
	Anything statistic;
	wpm.Statistic(statistic);
	expected["PoolSize"] = numberOfThreads;
	expected["TotalRequests"] = numberOfThreads * concurrencyFactor;
	assertAnyCompareEqual(expected, statistic, "Expected", '.', ':');
	(*pIos) << setfill(' ') << TimeStamp().AsString() << ' ' << setw(6) << statistic["TotalTime"].AsLong(0) << "ms " << strRemainder << flush;
}

void ThreadedTimeStampTest::CheckNumberOfRuns(long numberOfRuns, long doneRuns, String threadName)
{
	t_assert(numberOfRuns == doneRuns);
}

void ThreadedTimeStampTest::CheckPrepare2Run(bool isReady, bool wasPrepared)
{
}

void ThreadedTimeStampTest::CheckProcessWorkload(bool isWorking, bool wasPrepared)
{
}
