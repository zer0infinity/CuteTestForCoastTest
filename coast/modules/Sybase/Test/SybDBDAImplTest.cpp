/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Context.h"
#include "Mapper.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "SybDBDAImpl.h"

//--- interface include --------------------------------------------------------
#include "SybDBDAImplTest.h"

//---- SybDBDAImplTest ----------------------------------------------------------------
SybDBDAImplTest::SybDBDAImplTest(TString tstrName) : TestCase(name)
{
	StartTrace(SybDBDAImplTest.Ctor);
}

SybDBDAImplTest::~SybDBDAImplTest()
{
	StartTrace(SybDBDAImplTest.Dtor);
}

// setup for this TestCase
void SybDBDAImplTest::setUp ()
{
	StartTrace(SybDBDAImplTest.setUp);
}

void SybDBDAImplTest::tearDown ()
{
	StartTrace(SybDBDAImplTest.tearDown);
}

void SybDBDAImplTest::testCase()
{
	StartTrace(SybDBDAImplTest.testCase);

	Anything params;
	params["SybDBUser"] = "test";
	params["SybDBPW"] = "testtest";
	params["SybDBHost"] = "feadntsrv1";
	params["SybDBApp"] = "SybDBDAImplTest";
	params["SQL"] = "select au_fname, au_lname from authors where au_lname='Bennet'";

	TraceAny(params, "Input params:");
	Context ctx(params);

	Anything result;
	Anything expected;
	expected["Mapper"]["Query"] = params["SQL"];
	expected["Mapper"]["QueryCount"] = 1;
	expected["Mapper"]["QueryResult"]["0"]["au_fname"] = "Abraham";
	expected["Mapper"]["QueryResult"]["0"]["au_lname"] = "Bennet";
	TraceAny(expected, "expected tmpstore: ");
	// force load of configuration
	SybDBDAImpl da("SybSearchTestCoded");
	da.CheckConfig("DataAccessImpl");
	InputMapper	inpMapper("SybSearchTestCoded");
	inpMapper.CheckConfig("InputMapper");
	OutputMapper	outMapper("SybSearchTestCoded");
	outMapper.CheckConfig("OutputMapper");

	ctx.Push("DataAccess", &da);
	t_assert(da.Exec(ctx, &inpMapper, &outMapper));
	ctx.Remove("DataAccess");

	result = ctx.GetTmpStore();
	Anything cfg;
	cfg["Slot"] = "Mapper.Messages";
	SlotCleaner slc;
	slc.Operate(result, cfg);
	TraceAny(result, "resulting tmpstore: ");

	assertAnyEqual(expected, result);
}

// builds up a suite of testcases, add a line for each testmethod
Test *SybDBDAImplTest::suite ()
{
	StartTrace(SybDBDAImplTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(SybDBDAImplTest, testCase));

	return testSuite;

} // suite
