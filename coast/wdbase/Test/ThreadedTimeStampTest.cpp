/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ThreadedTimeStampTest.h"
#include "TimeStamp.h"
#include "TestSuite.h"
#include "TimeStampTestThread.h"
#include "AnyUtils.h"
#include "SystemLog.h"

ThreadedTimeStampTest::ThreadedTimeStampTest(TString tstrName)
	: TestCaseType(tstrName)
	, fCheckMutex("ThreadedTimeStampTest")
{
	StartTrace(ThreadedTimeStampTest.ThreadedTimeStampTest);
}

TString ThreadedTimeStampTest::getConfigFileName()
{
	return "ThreadedTimeStampTestConfig";
}

ThreadedTimeStampTest::~ThreadedTimeStampTest()
{
	StartTrace(ThreadedTimeStampTest.Dtor);
}

// builds up a suite of testcases, add a line for each testmethod
Test *ThreadedTimeStampTest::suite ()
{
	StartTrace(ThreadedTimeStampTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ThreadedTimeStampTest, TimestampConcurrencyTest);
	ADD_CASE(testSuite, ThreadedTimeStampTest, ExportCsvStatistics);
	return testSuite;
}

void ThreadedTimeStampTest::TimestampConcurrencyTest()
{
	StartTrace(ThreadedTimeStampTest.TimestampConcurrencyTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(roaConfig) ) {
		long numberOfRuns(roaConfig["NumberOfRuns"].AsLong());
		long numberOfThreads(roaConfig["NumberOfThreads"].AsLong());
		long concurrencyFactor(roaConfig["ConcurrencyFactor"].AsLong());
		DoTimeStampConcurrencyTest(numberOfRuns, numberOfThreads, concurrencyFactor, roaConfig);
	}
}

void ThreadedTimeStampTest::DoTimeStampConcurrencyTest(long numberOfRuns, long numberOfThreads, long concurrencyFactor, ROAnything roaConfig)
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
	t_assert(wpm.GetPoolSize() == numberOfThreads);
	t_assert(wpm.ResourcesUsed() == 0);
	String strRemainder;
	strRemainder << "Threads: " << (numberOfThreads > 9 ? "" : " ") << numberOfThreads << " Concurrency: " << concurrencyFactor << " Runs: " << numberOfRuns << " UTC-Test: " << (roaConfig["UTCCtorTest"].AsBool(false) ? "true " : "false") << " Compare: " << (roaConfig["CompareStamps"].AsBool(false) ? "true " : "false") << "\n";
	Trace(TimeStamp().AsString() << " Start    " << strRemainder);
	SystemLog::WriteToStderr(strRemainder);
	{
		CatchTimeType aTimer(TString("TimeStampConcurrencyTest/") << numberOfThreads << "t/" << (roaConfig["CompareStamps"].AsBool(false) ? "Compare" : "NoCompare") << "/" << (roaConfig["UTCCtorTest"].AsBool(false) ? "UTC/" : "Local/") << numberOfRuns, this);
		for (long i = 0; i < numberOfThreads * concurrencyFactor; i++) {
			wpm.Enter(config, -1L);
		}
		t_assert(wpm.AwaitEmpty(100));
	}
	Trace(TimeStamp().AsString() << " End      " << strRemainder);
	t_assert(wpm.Terminate());

	Anything expected;
	Anything statistic;
	wpm.Statistic(statistic);
	expected["PoolSize"] = numberOfThreads;
	expected["TotalRequests"] = numberOfThreads * concurrencyFactor;
	assertAnyCompareEqual(expected, statistic, "Expected", '.', ':');
}

void ThreadedTimeStampTest::CheckNumberOfRuns(long numberOfRuns, long doneRuns, String threadName)
{
	t_assert(numberOfRuns == doneRuns);
}

void ThreadedTimeStampTest::CheckProcessWorkload(bool isWorking, bool wasPrepared)
{
}
