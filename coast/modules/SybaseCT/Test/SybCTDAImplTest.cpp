/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTDAImplTest.h"

//--- module under test --------------------------------------------------------
#include "SybCTDAImpl.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "StringStream.h"
#include "AnyUtils.h"

//---- SybCTDAImplTest ----------------------------------------------------------------
SybCTDAImplTest::SybCTDAImplTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(SybCTDAImplTest.Ctor);
}

SybCTDAImplTest::~SybCTDAImplTest()
{
	StartTrace(SybCTDAImplTest.Dtor);
}

void SybCTDAImplTest::SimpleDATest()
{
	StartTrace(SybCTDAImplTest.SimpleDATest);

	Anything params;
	params["SybDBUser"] = "wdtester";
	params["SybDBPW"] = "all2test";
	params["SybDBHost"] = "HIKU_INT2";
	params["SybDBApp"] = "SybCTDAImplTest";
	params["SQL"] = "select au_fname, au_lname from authors where au_lname='Bennet'";

	TraceAny(params, "Input params:");
	Context ctx(params);

	Anything result;
	Anything expected;
	expected["Mapper"]["Query"] = params["SQL"];
	expected["Mapper"]["QueryCount"] = 1;
	expected["Mapper"]["QueryResult"]["0"]["au_fname"] = "Abraham";
	expected["Mapper"]["QueryResult"]["0"]["au_lname"] = "Bennet";
	expected["Mapper"]["QuerySource"] = "HIKU_INT2";
	TraceAny(expected, "expected tmpstore: ");
	// force load of configuration
	SybCTDAImpl da("SybSearchTestCoded");
	da.Initialize("SybCTDAImpl");
	ParameterMapper	inpMapper("SybSearchTestCoded");
	inpMapper.Initialize("ParameterMapper");
	ResultMapper	outMapper("SybSearchTestCoded");
	outMapper.Initialize("ResultMapper");

	ctx.Push("DataAccess", &da);
	t_assert(da.Exec(ctx, &inpMapper, &outMapper));
	ctx.Remove("DataAccess");

	result = ctx.GetTmpStore();
	Anything cfg;
	cfg["Slot"] = "Mapper.Messages";
	SlotCleaner::Operate(result, cfg);
	TraceAny(result, "resulting tmpstore: ");

	assertAnyCompareEqual(expected, result, "TmpStore.", '.', ':');
}

// builds up a suite of testcases, add a line for each testmethod
Test *SybCTDAImplTest::suite ()
{
	StartTrace(SybCTDAImplTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SybCTDAImplTest, SimpleDATest);
	return testSuite;
}
