/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StressAppTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "StressApp.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Stresser.h"
#include "StressProcessor.h"
#include "Dbg.h"

//---- StressAppTest ----------------------------------------------------------------
StressAppTest::StressAppTest(TString tstrName) : TestCaseType(tstrName) { }

StressAppTest::~StressAppTest()
{
}

void StressAppTest::AppRunTest()
{

	StressApp stressApp("TestedStressApp");
	stressApp.CheckConfig("Application");
	long ret = stressApp.Run();
	assertEqual(0, ret);
	Anything result = stressApp.fResult;
	assertEqual(10, result["Results"].GetSize());
	assertEqual(10, result["Total"]["Nr"].AsLong(0));
	assertEqual(500000, result["Total"]["Steps"].AsLong(0));
	assertEqual(1, result["Total"]["Max"].AsLong(0));
	assertEqual(1, result["Total"]["Min"].AsLong(0));
	assertEqual(0, result["Total"]["Error"].AsLong(2));
	CheckSum(result);
}

void StressAppTest::DataAccessStresserTest()
{
	StartTrace(StressAppTest.DataAccessStresserTest);
	Anything result = Stresser::RunStresser("Connection1");
	TraceAny(result, "Result");
	assertEqual(1, result[0L]["Nr"].AsLong(0));
	assertEqual(10, result[0L]["Steps"].AsLong(0));
}

void StressAppTest::ThreadedStresserRunnerTest()
{
	// Uses ten DummyStressers
	Anything result = Stresser::RunStresser("TStresserRunner");

	assertEqual(10, result["Results"].GetSize());
	assertEqual(10, result["Total"]["Nr"].AsLong(0));
	assertEqual(500000, result["Total"]["Steps"].AsLong(0));
	assertEqual(1, result["Total"]["Max"].AsLong(0));
	assertEqual(1, result["Total"]["Min"].AsLong(0));
	assertEqual(0, result["Total"]["Error"].AsLong(2));
	CheckSum(result);
}

void StressAppTest::FlowControlDAStresserTest()
{
	StartTrace(StressAppTest.FlowControlDAStresserTest);
	Anything result = Stresser::RunStresser("FlowControlled");
	TraceAny(result, "Results");
	assertEqual(26, result[0L]["Nr"].AsLong(0));
	assertEqual(26, result[0L]["Steps"].AsLong(0));
	assertEqual(0, result[0L]["Error"].AsLong(2));

	// Server not avail
	result = Stresser::RunStresser("FlowControlledDataAccessFailed");
	TraceAny(result, "Results");
	assertEqual(1, result[0L]["Nr"].AsLong(0));
	assertEqual(1, result[0L]["Steps"].AsLong(0));
	assertEqual(2, result[0L]["Error"].AsLong(0));
}

void StressAppTest::StressProcessorTest()
{
	// Uses ten DummyStressers
	StressProcessor stressProc("TestProcessor");
	Context ctx;
	ctx.GetEnvStore()["StresserName"] = "TStresserRunner";
	String theString;
	OStringStream output(&theString);
	stressProc.DoProcessRequest(output, ctx);
	output.flush();
	IStringStream intoAny(theString);
	Anything result;
	result.Import(intoAny);

	assertEqual(10, result["Results"].GetSize());
	assertEqual(10, result["Total"]["Nr"].AsLong(0));
	assertEqual(500000, result["Total"]["Steps"].AsLong(0));
	assertEqual(1, result["Total"]["Max"].AsLong(0));
	assertEqual(1, result["Total"]["Min"].AsLong(0));
	assertEqual(0, result["Total"]["Error"].AsLong(2));
	CheckSum(result);
}

void StressAppTest::CheckSum(ROAnything result)
{
	StartTrace(StressAppTest.CheckSum);
	long checkSum(0);
	TraceAny(result, "results");
	ROAnything results = result["Results"];
	long sz = results.GetSize();
	for (long i = 0; i < sz; i++) {
		checkSum += results[i]["Sum"].AsLong(-1);
	}
	assertEqual(checkSum, result["Total"]["Sum"].AsLong(0));
}

Test *StressAppTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, StressAppTest, AppRunTest);
	ADD_CASE(testSuite, StressAppTest, ThreadedStresserRunnerTest);
	ADD_CASE(testSuite, StressAppTest, FlowControlDAStresserTest);
	ADD_CASE(testSuite, StressAppTest, DataAccessStresserTest);
	ADD_CASE(testSuite, StressAppTest, StressProcessorTest);

	return testSuite;
}
