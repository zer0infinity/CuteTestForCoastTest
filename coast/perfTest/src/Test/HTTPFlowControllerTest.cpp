/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "AnyUtils.h"
#include "Context.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "HttpFlowController.h"

//--- interface include --------------------------------------------------------
#include "HTTPFlowControllerTest.h"

//---- HTTPFlowControllerTest ----------------------------------------------------------------
HTTPFlowControllerTest::HTTPFlowControllerTest(TString tstrName) : ConfiguredActionTest(tstrName, "HTTPFlowControllerTestConfig"),
	fCtx()
{ }

HTTPFlowControllerTest::HTTPFlowControllerTest(TString tstrName, TString configFileName) : ConfiguredActionTest(tstrName, configFileName),
	fCtx()
{ }

HTTPFlowControllerTest::~HTTPFlowControllerTest()
{
}

// Checks if the tmpStore is prepared according the FlowController's configuration
void HTTPFlowControllerTest::DoTest(Anything testCase, const char *testCaseName)
{
	StartTrace1(HTTPFlowControllerTest.DoTest, "<" << testCaseName << ">");

	DoTestWithContext(testCase, testCaseName, fCtx);
	DoCheckStores(testCase["Result"], fCtx, testCaseName);

}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPFlowControllerTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HTTPFlowControllerTest, RunTestCases));

	return testSuite;

} // suite
