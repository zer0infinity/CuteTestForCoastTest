/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "DummyTest.h"
//--- standard modules used ----------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------

DummyTest::DummyTest(TString tstrName)
	: TestCaseType(tstrName)
{}

DummyTest::~DummyTest()
{}

void DummyTest::dummyTest ()
{
	// Just a few test cases to show how it works
	t_assert(1);
	assertEqual(1, 1);
	assertDoublesEqual(1.2, 1.2000000001, 1E-5);
	assertEqual("Test", "Test");
	t_assert(0);
	assertEqual(0, 1);
	assertEqual("Null", "Eins");
	t_assertm(0, "Hi, I just wanted to fail");
	assertEqualm(0, 1, "Hi, I just wanted to fail");
	assertEqualm("Null", "Eins", "Hi, I just wanted to fail");
}

Test *DummyTest::suite ()
{
	// what: return the whole suite of tests for DummyTest, add all top level test functions here.
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, DummyTest, dummyTest);
	return testSuite;
}
