/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SlotnameOutputMapperTest.h"
#include "SlotnameOutputMapper.h"
#include "TestSuite.h"

//---- SlotnameOutputMapperTest ----------------------------------------------------------------
SlotnameOutputMapperTest::SlotnameOutputMapperTest(TString tname)
	: ConfiguredActionTest(tname)
{
	StartTrace(SlotnameOutputMapperTest.SlotnameOutputMapperTest);
}

TString SlotnameOutputMapperTest::getConfigFileName()
{
	return "SlotnameOutputMapperTestConfig";
}

SlotnameOutputMapperTest::~SlotnameOutputMapperTest()
{
	StartTrace(SlotnameOutputMapperTest.Dtor);
}

void SlotnameOutputMapperTest::BasicFunctionTest()
{
	StartTrace(SlotnameOutputMapperTest.BasicFunctionTest);

	SlotnameOutputMapper som("SlotnameTestOutputMapper");
	som.Initialize("ResultMapper");

	Context ctx;
	som.Put("Data", GetTestCaseConfig()["AnyToPut"].DeepClone(), ctx);
	CheckStores(GetTestCaseConfig()["Result"], ctx, name());
}

void SlotnameOutputMapperTest::OverwriteOrAppendTest()
{
	StartTrace(SlotnameOutputMapperTest.OverwriteOrAppendTest);

	SlotnameOutputMapper som("SlotnameTestOutputMapper");
	som.Initialize("ResultMapper");

	Context ctx;
	som.Put("Data", GetTestCaseConfig()["AnyToPut"][0L].DeepClone(), ctx);
	som.Put("Data", GetTestCaseConfig()["AnyToPut"][1L].DeepClone(), ctx);
	CheckStores(GetTestCaseConfig()["Result"], ctx, name());
}

// builds up a suite of tests, add a line for each testmethod
Test *SlotnameOutputMapperTest::suite ()
{
	StartTrace(SlotnameOutputMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SlotnameOutputMapperTest, BasicFunctionTest);
	ADD_CASE(testSuite, SlotnameOutputMapperTest, OverwriteOrAppendTest);
	return testSuite;
}
