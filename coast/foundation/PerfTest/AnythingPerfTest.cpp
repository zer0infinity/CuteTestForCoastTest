/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnythingPerfTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Dbg.h"
#include "PoolAllocator.h"
#include "System.h"

//---- AnythingPerfTest ----------------------------------------------------------------
AnythingPerfTest::AnythingPerfTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(AnythingPerfTest.Ctor);
}

AnythingPerfTest::~AnythingPerfTest()
{
	StartTrace(AnythingPerfTest.Dtor);
}

void AnythingPerfTest::RunIndexLoopAsCharPtr(long index, const Anything &a, const long iterations)
{
	CatchTimeType aTimer(TString("IndexLoop/AsCharPtr/") << iterations, this, '/');
	String out;
	for (long i = 0; i < iterations; ++i) {
		out = a[index].AsCharPtr("lookup hallo");
	}
}

void AnythingPerfTest::RunIndexLoopAsString(long index, const Anything &a, const long iterations)
{
	CatchTimeType aTimer(TString("IndexLoop/AsString/") << iterations, this, '/');
	String out;
	for (long i = 0; i < iterations; ++i) {
		out = a[index].AsString("lookup hallo");
	}
}

void AnythingPerfTest::RunKeyLoop(const char *key, const Anything &a, const long iterations)
{
	CatchTimeType aTimer(TString("KeyLoop/AsCharPtr/[") << key << "]/" << iterations, this, '/');
	String out;
	for (long i = 0; i < iterations; ++i) {
		out = a[key].AsCharPtr("lookup hallo");
	}
}

void AnythingPerfTest::RunLookupPathLoop(const char *key, const Anything &a, const long iterations)
{
	RunROLookupPathLoop(key, a, iterations);
	CatchTimeType aTimer(TString("LookupPathLoop/") << key << '/' << iterations << "/any", this, '/');
	const char *out;
	Anything result;
	for (long i = 0; i < iterations; ++i) {
		a.LookupPath(result, key);
		out = result.AsCharPtr("lookup hallo");
	}
}

void AnythingPerfTest::RunROLookupPathLoop(const char *key, const ROAnything &a, const long iterations)
{
	CatchTimeType aTimer(TString("LookupPathLoop/") << key << '/' << iterations << "/roa", this, '/');
	const char *out;
	ROAnything result;
	for (long i = 0; i < iterations; ++i) {
		a.LookupPath(result, key);
		out = result.AsCharPtr("lookup hallo");
	}
}

void AnythingPerfTest::LookupTest()
{
	StartTrace(AnythingPerfTest.LookupTest);
	PoolAllocator p(1, 16384, 10);

	TestStorageHooks tsh(&p);

	Anything a(&p);
	a["key"] = "foo";
	a["key"]["which"]["is"]["more"]["complex"] = 1L;
	const long iterations = 100000;
	RunIndexLoopAsCharPtr(0L, a, iterations);
	RunIndexLoopAsString(0L, a, iterations);
	RunKeyLoop("key", a, iterations);
	RunKeyLoop("keywhichissomewhatlonger", a, iterations);
	RunLookupPathLoop("key", a, iterations);
	RunLookupPathLoop("keywhichissomewhatlonger", a, iterations);
	RunLookupPathLoop("key/which/is/not/there", a, iterations);
	RunLookupPathLoop("key/which/is/more/complex", a, iterations);
	// now with a more populated Anything!
	Anything b(&p);
	b["name"] = "Peter";
	b["mnam"] = "Christian";
	b["snam"] = "Sommerlad";
	b["road"] = "Erlenstrasse";
	b["hsnr"] = 79;
	b["adrs"]["postleitzahl"] = 8832;
	b["adrs"]["Ortsnamentag"] = "Wollerau";
	RunKeyLoop("hsnr", b, iterations);
	RunKeyLoop("name", b, iterations);
	RunKeyLoop("noth", b, iterations);
	RunKeyLoop("adrs", b, iterations);
	RunLookupPathLoop("adrs", b, iterations);
	RunLookupPathLoop("adrs/postleitzahl", b, iterations);
	RunLookupPathLoop("adrs/Ortsnamentag", b, iterations);
	RunLookupPathLoop("adrs/nottheretag", b, iterations);
	RunLookupPathLoop("adrs[0]", b, iterations);
	RunLookupPathLoop("adrs[10]", b, iterations);
	RunLookupPathLoop("1[10]", b, iterations);

	t_assertm(true, "dummy assertion to generate summary output");
}

void AnythingPerfTest::DeepCloneTest()
{
	StartTrace(AnythingPerfTest.DeepCloneTest);
	DoFunctorTest(1L, "long 1L", &AnythingPerfTest::RunDeepCloneLoop);
	DoFunctorTest(0.1234, "float 0.1234", &AnythingPerfTest::RunDeepCloneLoop);
	DoFunctorTest(4433.1234, "float 4433.1234", &AnythingPerfTest::RunDeepCloneLoop);
	DoFunctorTest("guguseli", "char* guguseli", &AnythingPerfTest::RunDeepCloneLoop);
	DoFunctorTest(String("guguseli"), "String guguseli", &AnythingPerfTest::RunDeepCloneLoop);
}

void AnythingPerfTest::PrintOnTest()
{
	StartTrace(AnythingPerfTest.PrintOnTest);
	DoFunctorTest(1L, "long 1L", &AnythingPerfTest::RunPrintOnPrettyLoop);
	DoFunctorTest(0.1234, "float 0.1234", &AnythingPerfTest::RunPrintOnPrettyLoop);
	DoFunctorTest(4433.1234, "float 4433.1234", &AnythingPerfTest::RunPrintOnPrettyLoop);
	DoFunctorTest("guguseli", "char* guguseli", &AnythingPerfTest::RunPrintOnPrettyLoop);
	DoFunctorTest(String("guguseli"), "String guguseli", &AnythingPerfTest::RunPrintOnPrettyLoop);
}

template <typename T>
void AnythingPerfTest::DoFunctorTest(T value, const char *pName, LoopFunctor pFunc)
{
	StartTrace(AnythingPerfTest.DoFunctorTest);
	PoolAllocator p(1, 32768, 20);

	TestStorageHooks tsh(&p);

	const long iterations = 10000;

	{
		Anything a(&p);
		(this->*pFunc)("Null-Any", a, iterations);
	}
	{
		MetaThing m(&p);
		(this->*pFunc)("MetaThing", m, iterations);
	}
//	p.Refresh();
	{
		Anything a(&p);
		a["key"] = value;
		(this->*pFunc)(String("1 any ") << pName, a, iterations);
	}
//	p.Refresh();
	{
		Anything a(&p);
		a["key1"] = value;
		a["key2"] = value;
		(this->*pFunc)(String("2 any ") << pName, a, iterations);
	}
//	p.Refresh();
	{
		Anything a(&p);
		a["key"]["second"] = value;
		(this->*pFunc)(String("11 any ") << pName, a, iterations);
	}
	p.Refresh();
	{
		Anything a(&p);
		a["key"]["second"] = value;
		a["key"]["third"] = value;
		(this->*pFunc)(String("12 any ") << pName, a, iterations);
	}
//	p.Refresh();
	{
		Anything a(&p);
		a["key1"]["second"] = value;
		a["key1"]["third"] = value;
		a["key2"]["second"] = value;
		a["key2"]["third"] = value;
		(this->*pFunc)(String("22 any ") << pName, a, iterations);
	}
	t_assertm(true, "dummy assertion to generate summary output");
}

void AnythingPerfTest::RunDeepCloneLoop(const char *pName, const Anything &a, const long iterations)
{
	CatchTimeType aTimer(TString("DeepCloneLoop/[") << pName << "]/" << iterations, this, '/');
	Anything result;
	for (long i = 0; i < iterations; ++i) {
		result = a.DeepClone();
	}
}

void AnythingPerfTest::RunPrintOnPrettyLoop(const char *pName, const Anything &a, const long iterations)
{
	CatchTimeType aTimer(TString("PrintOnPrettyLoop/[") << pName << "]/" << iterations, this, '/');
	String strBuf;
	OStringStream stream(&strBuf);
	for (long i = 0; i < iterations; ++i) {
		a.PrintOn(stream, true);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *AnythingPerfTest::suite ()
{
	StartTrace(AnythingPerfTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AnythingPerfTest, LookupTest);
	ADD_CASE(testSuite, AnythingPerfTest, DeepCloneTest);
	ADD_CASE(testSuite, AnythingPerfTest, PrintOnTest);
	ADD_CASE(testSuite, AnythingPerfTest, ExportCsvStatistics);

	return testSuite;
}
