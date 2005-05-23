/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "SybDB.h"

//--- interface include --------------------------------------------------------
#include "SybDBTest.h"

//---- SybDBTest ----------------------------------------------------------------
SybDBTest::SybDBTest(TString tstrName) : TestCase(name)
{
	StartTrace(SybDBTest.Ctor);
}

SybDBTest::~SybDBTest()
{
	StartTrace(SybDBTest.Dtor);
}

// setup for this TestCase
void SybDBTest::setUp ()
{
	StartTrace(SybDBTest.setUp);
} // setUp

void SybDBTest::tearDown ()
{
	StartTrace(SybDBTest.tearDown);
} // tearDown

void SybDBTest::testSybDBTest()
{
	StartTrace(SybDBTest.testSybDBTest);
	SybDB sybdb;

	if (t_assertm(sybdb.Open("test", "testtest", "feadntsrv1", "testSybDBTest"), "dbOpen should have succeeded")) {
		if (t_assert(sybdb.SqlExec("use pubs2"))) {
			if (t_assert(sybdb.SqlExec("select * from authors"))) {
				Anything result;
				if (t_assert(sybdb.GetResult(result))) {
					t_assertm(result.GetSize() == 23, "expected 23 rows");
				}
			} else {
				Trace("SqlExec2 not successful!");
			}
		} else {
			Trace("SqlExec1 not successful!");
		}
		sybdb.Close();
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *SybDBTest::suite ()
{
	StartTrace(SybDBTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(SybDBTest, testSybDBTest));

	return testSuite;

} // suite
