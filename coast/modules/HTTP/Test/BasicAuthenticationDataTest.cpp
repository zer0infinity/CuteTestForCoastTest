/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "BasicAuthenticationDataTest.h"
#include "BasicAuthenticationData.h"

//--- module under test --------------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- BasicAuthenticationDataTest ----------------------------------------------------------------
BasicAuthenticationDataTest::BasicAuthenticationDataTest(TString name)
	: ConfiguredTestCase(name, "BasicAuthenticationDataTestConfig")
{
	StartTrace(BasicAuthenticationDataTest.Ctor);
}

BasicAuthenticationDataTest::~BasicAuthenticationDataTest()
{
	StartTrace(BasicAuthenticationDataTest.Dtor);
}

// setup for this ConfiguredTestCase
void BasicAuthenticationDataTest::setUp ()
{
	StartTrace(BasicAuthenticationDataTest.setUp);
	ConfiguredTestCase::setUp();
}

void BasicAuthenticationDataTest::tearDown ()
{
	StartTrace(BasicAuthenticationDataTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void BasicAuthenticationDataTest::test()
{
	StartTrace(BasicAuthenticationDataTest.test);

	FOREACH_ENTRY("TestCases", cConfig, cName) {
		BasicAuthenticationData bad(cConfig["Input"].AsString());

		bool expCorrect = cConfig["Correct"].AsBool();
		t_assertm(expCorrect == bad.IsSyntaxValid(), cName);
		if (expCorrect) {
			assertEqualm(cConfig["UserName"].AsString(), bad.GetUserName(), cName);
			assertEqualm(cConfig["Password"].AsString(), bad.GetPassword(), cName);
		}
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *BasicAuthenticationDataTest::suite ()
{
	StartTrace(BasicAuthenticationDataTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, BasicAuthenticationDataTest, test);

	return testSuite;
}
