/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "UniqueIdGenTest.h"
#include "UniqueIdGen.h"
#include "Dbg.h"
#include "Anything.h"

//---- UniqueIdGenTest ----------------------------------------------------------------
UniqueIdGenTest::UniqueIdGenTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(UniqueIdGenTest.Ctor);
}

UniqueIdGenTest::~UniqueIdGenTest()
{
	StartTrace(UniqueIdGenTest.Dtor);
}

void UniqueIdGenTest::setUp ()
{
	StartTrace(UniqueIdGenTest.setUp);
}

void UniqueIdGenTest::tearDown ()
{
	StartTrace(UniqueIdGenTest.tearDown);
}

void UniqueIdGenTest::GetUniqueIdTest()
{
	StartTrace(UniqueIdGenTest.GetUniqueIdTest);
	long iterations = 20000L;

	DoGetUniqueIdTest(iterations, "");
	DoGetUniqueIdTest(iterations, "xxx");
}

void UniqueIdGenTest::DoGetUniqueIdTest(long iterations, const String &additionalToken)
{
	StartTrace(UniqueIdGenTest.DoGetUniqueIdTest);
	Anything uniqueId, check;

	for ( long l = 0; l < iterations; l++ ) {
		uniqueId[l] = UniqueIdGen::GetUniqueId(additionalToken);
		Trace("uniqueId :[" << uniqueId[l].AsString() << "]");
	}
	for ( long l = 0; l < iterations; l++ ) {
		Trace(uniqueId[l].AsString());
		check[uniqueId[l].AsString()]  = check[uniqueId[l].AsString()].AsLong(0) + 1L;
	}
	// We should get 10000 unique ID's
	assertEqual(iterations, check.GetSize());
	for ( long l = 0; l < iterations; l++ ) {
		assertEqual(1L, check[l].AsLong(0L));
	}
	TraceAny(uniqueId, "uniqueId");
}

// builds up a suite of testcases, add a line for each testmethod
Test *UniqueIdGenTest::suite ()
{
	StartTrace(UniqueIdGenTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, UniqueIdGenTest, GetUniqueIdTest);
	return testSuite;
}
