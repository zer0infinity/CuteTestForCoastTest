/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "FlowController.h"

//--- interface include --------------------------------------------------------
#include "FlowControllerTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- FlowControllerTest ----------------------------------------------------------------
FlowControllerTest::FlowControllerTest(TString tstrName) : StressAppTest(tstrName) { }

FlowControllerTest::~FlowControllerTest()
{
}

// setup for this TestCase
void FlowControllerTest::setUp ()
{
	StressAppTest::setUp();

	istream *ifp = System::OpenStream("FlowControllerTestConfig", "any");
	if (ifp == 0) {
		String logMsg;
		cerr << logMsg << "FlowControllerTest:setUp : can't open file FlowControllerTestConfig.any" << endl;
		return;
	}
	fConfig.Import(*ifp);
	delete ifp;

} // setUp

void FlowControllerTest::tearDown ()
{
} // tearDown

//Test with Full configuration
void FlowControllerTest::FullConfigurationTest()
{
	DoTest();
}

//Test with configuration without NumberOfRuns ( default of one is taken )
void FlowControllerTest::WithoutNumberOfRunsConfigurationTest()
{
	DoTest();
}

//Test with configuration with NumberOfRuns=0 ( run is not executed )
void FlowControllerTest::NumberOfRuns0ConfigurationTest()
{
	DoTest();
}

//Test with configuration without PreRun (just Run executed )
void FlowControllerTest::WithoutPreRunConfigurationTest()
{
	DoTest();
}

//Test with configuration without Run ( just PreRun executed )
void FlowControllerTest::WithoutRunConfigurationTest()
{
	DoTest();
}

//Test with configuration with only PreRun
void FlowControllerTest::PreRunOnlyConfigurationTest()
{
	DoTest();
}

//Test with configuration with only Run
void FlowControllerTest::RunOnlyConfigurationTest()
{
	DoTest();
}

//Test with configuration with only Run and only one step
void FlowControllerTest::RunOnlyOneStepConfigurationTest()
{
	DoTest();
}

//Test with configuration that does not contain any of the necessary slots
void FlowControllerTest::InvalidConfigurationTest()
{
	DoTest();
}

// Checks if the tmpStore is prepared according the FlowController's configuration
void FlowControllerTest::DoTest()
{
	StartTrace1(FlowControllerTest.DoTest, "<" << fName << ">");

	String controllerName = fConfig[fName]["FlowController"].AsString("Invalid");
	FlowController fc(controllerName);
	fc.CheckConfig("FlowController");

	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0);

	Anything tmpStore = ctx.GetTmpStore();
	Anything results = fConfig[fName]["Results"];
	long sz = results.GetSize();
	for (long i = 0; i < sz; i++) {
		t_assert(fc.PrepareRequest(ctx));
		TraceAny(tmpStore, "TempStore");
		Anything actualResult = results[i];
		long subsz = actualResult.GetSize();
		for (long j = 0; j < subsz; j++) {
			String slotname = actualResult.SlotName(j);
			assertAnyEqual(actualResult[j], tmpStore[slotname]);
		}
	}
	// Check if really finished
	t_assert(!fc.PrepareRequest(ctx));
}

// builds up a suite of testcases, add a line for each testmethod
Test *FlowControllerTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(FlowControllerTest, FullConfigurationTest));
	testSuite->addTest (NEW_CASE(FlowControllerTest, NumberOfRuns0ConfigurationTest));
	testSuite->addTest (NEW_CASE(FlowControllerTest, WithoutNumberOfRunsConfigurationTest));
	testSuite->addTest (NEW_CASE(FlowControllerTest, WithoutPreRunConfigurationTest));
	testSuite->addTest (NEW_CASE(FlowControllerTest, WithoutRunConfigurationTest));
	testSuite->addTest (NEW_CASE(FlowControllerTest, RunOnlyConfigurationTest));
	testSuite->addTest (NEW_CASE(FlowControllerTest, RunOnlyOneStepConfigurationTest));
	testSuite->addTest (NEW_CASE(FlowControllerTest, PreRunOnlyConfigurationTest));
	testSuite->addTest (NEW_CASE(FlowControllerTest, InvalidConfigurationTest));

	return testSuite;

} // suite
