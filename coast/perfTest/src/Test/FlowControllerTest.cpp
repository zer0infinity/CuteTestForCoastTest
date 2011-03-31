/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "FlowController.h"
#include "FlowControllerTest.h"
#include "SystemFile.h"
#include "Dbg.h"
#include <iostream>

//---- FlowControllerTest ----------------------------------------------------------------
FlowControllerTest::FlowControllerTest(TString tstrName) : StressAppTest(tstrName) { }

FlowControllerTest::~FlowControllerTest()
{
}

void FlowControllerTest::setUp ()
{
	StressAppTest::setUp();

	std::istream *ifp = Coast::System::OpenStream("FlowControllerTestConfig", "any");
	if (ifp == 0) {
		String logMsg;
		std::cerr << logMsg << "FlowControllerTest:setUp : can't open file FlowControllerTestConfig.any" << std::endl;
		return;
	}
	fConfig.Import(*ifp);
	delete ifp;

}

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
	fc.Initialize("FlowController");

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

	ADD_CASE(testSuite, FlowControllerTest, FullConfigurationTest);
	ADD_CASE(testSuite, FlowControllerTest, NumberOfRuns0ConfigurationTest);
	ADD_CASE(testSuite, FlowControllerTest, WithoutNumberOfRunsConfigurationTest);
	ADD_CASE(testSuite, FlowControllerTest, WithoutPreRunConfigurationTest);
	ADD_CASE(testSuite, FlowControllerTest, WithoutRunConfigurationTest);
	ADD_CASE(testSuite, FlowControllerTest, RunOnlyConfigurationTest);
	ADD_CASE(testSuite, FlowControllerTest, RunOnlyOneStepConfigurationTest);
	ADD_CASE(testSuite, FlowControllerTest, PreRunOnlyConfigurationTest);
	ADD_CASE(testSuite, FlowControllerTest, InvalidConfigurationTest);

	return testSuite;

}
