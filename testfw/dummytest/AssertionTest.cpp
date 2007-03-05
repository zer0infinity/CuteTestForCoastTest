/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AssertionTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------

//--- standard modules used ----------------------------------------------------

//--- c-modules used -----------------------------------------------------------

//---- AssertionTest ----------------------------------------------------------------
AssertionTest::AssertionTest(TString tstrName)
	: TestCaseType(tstrName)
{
}

AssertionTest::~AssertionTest()
{
}

// builds up a suite of testcases, add a line for each testmethod
Test *AssertionTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AssertionTest, MyCompareTest);
	return testSuite;
}
