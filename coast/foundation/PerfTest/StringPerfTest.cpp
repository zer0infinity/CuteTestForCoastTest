/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- interface include --------------------------------------------------------
#include "StringPerfTest.h"

//--- standard modules used ----------------------------------------------------
#include "DiffTimer.h"
#include "PoolAllocator.h"
#include "StringStream.h"
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- StringPerfTest ----------------------------------------------------------------
StringPerfTest::StringPerfTest(TString tstrName) : StatisticTestCase(tstrName)
{
	StartTrace(StringPerfTest.Ctor);
}

StringPerfTest::~StringPerfTest()
{
	StartTrace(StringPerfTest.Dtor);
}

// setup for this TestCase
void StringPerfTest::setUp()
{
	StartTrace(StringPerfTest.setUp);
	StatisticTestCase::setUp();
}

void StringPerfTest::tearDown()
{
	StartTrace(StringPerfTest.tearDown);
	StatisticTestCase::tearDown();
}

void StringPerfTest::RunLoop(const char *str, const long iterations)
{
	DiffTimer dt;
	String out;
	for (long i = 0; i < iterations; i++) {
		out << str;
	}
	long lDiff = dt.Diff();
	String strName("RunLoop[");
	AddStatisticOutput(strName << str << ']' << iterations, lDiff);
}

void StringPerfTest::RunPreallocLoop(const char *str, const long iterations)
{
	DiffTimer dt;
	String out(strlen(str)*iterations + 1);
	for (long i = 0; i < iterations; i++) {
		out << str;
	}
	long lDiff = dt.Diff();
	String strName("RunPreallocLoop[");
	AddStatisticOutput(strName << str << ']' << iterations, lDiff);
}

void StringPerfTest::RunPoolAllocLoop(const char *str, const long iterations)
{
	DiffTimer dt;
	PoolAllocator p(1, ((strlen(str)*iterations + 1) / 256 + 100), 21);
	String out( &p );
	for (long i = 0; i < iterations; i++) {
		out << str;
	}
	long lDiff = dt.Diff();
	String strName("RunPoolAllocLoop[");
	AddStatisticOutput(strName << str << ']' << iterations, lDiff);
}

void StringPerfTest::referenceTest()
{
	StartTrace(StringPerfTest.referenceTest);
	const char *sample = "this is a "; //short samp"; // 20 bytes
	const char *sample1 = "this is a short samp"; //this is a short samp"; // 40 bytes
	const long iterations = 100000;

	// short sample
	RunLoop(sample, iterations);
	RunPreallocLoop(sample, iterations);
	System::MicroSleep(1000L);
	RunPoolAllocLoop(sample, iterations);

	// long sample
	RunLoop(sample1, iterations);
	RunPreallocLoop(sample1, iterations);
	System::MicroSleep(1000L);
	RunPoolAllocLoop(sample1, iterations);
	t_assertm(true, "dummy assertion to generate summary output");
}

void StringPerfTest::ExportCsvStatistics()
{
	StatisticTestCase::ExportCsvStatistics(1L);
}

// builds up a suite of testcases, add a line for each testmethod
Test *StringPerfTest::suite()
{
	StartTrace(StringPerfTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, StringPerfTest, referenceTest);
	ADD_CASE(testSuite, StringPerfTest, ExportCsvStatistics);

	return testSuite;
}
