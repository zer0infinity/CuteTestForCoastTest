/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTPoolDAImplTest.h"

//--- module under test --------------------------------------------------------
#include "SybCTPoolDAImpl.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "StringStream.h"
#include "AnyUtils.h"

//---- SybCTPoolDAImplTest ----------------------------------------------------------------
SybCTPoolDAImplTest::SybCTPoolDAImplTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(SybCTPoolDAImplTest.Ctor);
}

SybCTPoolDAImplTest::~SybCTPoolDAImplTest()
{
	StartTrace(SybCTPoolDAImplTest.Dtor);
}

void SybCTPoolDAImplTest::SimpleDATest()
{
	StartTrace(SybCTPoolDAImplTest.SimpleDATest);

	Anything params;
	params["SybDBUser"] = "wdtester";
	params["SybDBPW"] = "all2test";
	params["SybDBHost"] = "HIKU_INT2";
	params["SybDBApp"] = "SybCTPoolDAImplTest";
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
	SybCTPoolDAImpl da("SybSearchTestCoded");
	da.CheckConfig("DataAccessImpl");
	ParameterMapper	inpMapper("SybSearchTestCoded");
	inpMapper.CheckConfig("ParameterMapper");
	ResultMapper	outMapper("SybSearchTestCoded");
	outMapper.CheckConfig("ResultMapper");

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
Test *SybCTPoolDAImplTest::suite ()
{
	StartTrace(SybCTPoolDAImplTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SybCTPoolDAImplTest, SimpleDATest);
	return testSuite;
}
