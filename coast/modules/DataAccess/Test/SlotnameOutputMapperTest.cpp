/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SlotnameOutputMapperTest.h"

//--- module under test --------------------------------------------------------
#include "SlotnameOutputMapper.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- SlotnameOutputMapperTest ----------------------------------------------------------------
SlotnameOutputMapperTest::SlotnameOutputMapperTest(TString tname)
	: ConfiguredActionTest(tname, "SlotnameOutputMapperTestConfig")
{
	StartTrace(SlotnameOutputMapperTest.Ctor);
}

SlotnameOutputMapperTest::~SlotnameOutputMapperTest()
{
	StartTrace(SlotnameOutputMapperTest.Dtor);
}

void SlotnameOutputMapperTest::BasicFunctionTest()
{
	StartTrace(SlotnameOutputMapperTest.BasicFunctionTest);

	SlotnameOutputMapper som("SlotnameTestOutputMapper");
	som.CheckConfig("OutputMapper");

	Context ctx;
	som.Put("Data", fTestCaseConfig["AnyToPut"], ctx);
	DoCheckStores(fTestCaseConfig["Result"], ctx, name());
}

void SlotnameOutputMapperTest::OverwriteOrAppendTest()
{
	StartTrace(SlotnameOutputMapperTest.OverwriteOrAppendTest);

	SlotnameOutputMapper som("SlotnameTestOutputMapper");
	som.CheckConfig("OutputMapper");

	Context ctx;
	som.Put("Data", fTestCaseConfig["AnyToPut"][0L], ctx);
	som.Put("Data", fTestCaseConfig["AnyToPut"][1L], ctx);
	DoCheckStores(fTestCaseConfig["Result"], ctx, name());
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *SlotnameOutputMapperTest::suite ()
{
	StartTrace(SlotnameOutputMapperTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SlotnameOutputMapperTest, BasicFunctionTest);
	ADD_CASE(testSuite, SlotnameOutputMapperTest, OverwriteOrAppendTest);

	return testSuite;

} // suite
