/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-modules used -----------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- project modules used -----------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ROAnyLookupAdapter.h"

//--- interface include --------------------------------------------------------
#include "ROAnyLookupAdapterTest.h"

//---- ROAnyLookupAdapterTest ----------------------------------------------------------------
ROAnyLookupAdapterTest::ROAnyLookupAdapterTest(TString name) : TestCase(name)
{
	StartTrace(ROAnyLookupAdapterTest.Ctor);
}

ROAnyLookupAdapterTest::~ROAnyLookupAdapterTest()
{
	StartTrace(ROAnyLookupAdapterTest.Dtor);
}

// setup for this TestCase
void ROAnyLookupAdapterTest::setUp ()
{
	StartTrace(ROAnyLookupAdapterTest.setUp);
}

void ROAnyLookupAdapterTest::tearDown ()
{
	StartTrace(ROAnyLookupAdapterTest.tearDown);
}

void ROAnyLookupAdapterTest::testCase()
{
	StartTrace(ROAnyLookupAdapterTest.testCase);
}

void ROAnyLookupAdapterTest::testLookup()
{
	StartTrace(ROAnyLookupAdapterTest.testLookup);

	Anything base;

	base["foo"] = "bar";
	base["long"] = 31416L;
	base["compo"]["site"] = "bar";

	ROAnything robase(base);
	ROAnyLookupAdapter la(robase);

	assertEquals("bar", la.Lookup("foo", "X"));
	assertEquals("bar", la.Lookup("compo@site", "X", '@'));

	t_assert(31416L == la.Lookup("long", 0L));

	assertEquals("X", la.Lookup("fox", "X"));
}

// builds up a suite of testcases, add a line for each testmethod
Test *ROAnyLookupAdapterTest::suite ()
{
	StartTrace(ROAnyLookupAdapterTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ROAnyLookupAdapterTest, testCase);
	ADD_CASE(testSuite, ROAnyLookupAdapterTest, testLookup);

	return testSuite;
}
