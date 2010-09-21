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
#include "PoolAllocator.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- StringPerfTest ----------------------------------------------------------------
StringPerfTest::StringPerfTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(StringPerfTest.Ctor);
}

StringPerfTest::~StringPerfTest()
{
	StartTrace(StringPerfTest.Dtor);
}

void StringPerfTest::RunLoop(const char *str, const long iterations)
{
	CatchTimeType aTimer(TString("RunLoop/") << str << '/' << iterations << "/default", this);
	String out;
	for (long i = 0; i < iterations; ++i) {
		out << str;
	}
}

void StringPerfTest::RunPreallocLoop(const char *str, const long iterations)
{
	CatchTimeType aTimer(TString("RunLoop/") << str << '/' << iterations << "/Prealloc", this);
	String out(strlen(str)*iterations + 1);
	for (long i = 0; i < iterations; ++i) {
		out << str;
	}
}

void StringPerfTest::RunPoolAllocLoop(const char *str, const long iterations)
{
	CatchTimeType aTimer(TString("RunLoop/") << str << '/' << iterations << "/PoolAlloc", this);
	PoolAllocator p(1, ((strlen(str) + 16)*iterations * 3 / 1024), 21);
	String out( &p );
	for (long i = 0; i < iterations; ++i) {
		out << str;
	}
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
	Coast::System::MicroSleep(1000L);
	RunPoolAllocLoop(sample, iterations);

	// long sample
	RunLoop(sample1, iterations);
	RunPreallocLoop(sample1, iterations);
	Coast::System::MicroSleep(1000L);
	RunPoolAllocLoop(sample1, iterations);
	t_assertm(true, "dummy assertion to generate summary output");
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
