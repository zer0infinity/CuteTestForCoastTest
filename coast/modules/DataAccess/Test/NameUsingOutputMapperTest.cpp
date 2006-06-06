/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "NameUsingOutputMapperTest.h"
//--- module under test --------------------------------------------------------
#include "NameUsingOutputMapper.h"
//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "Context.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//---- NameUsingOutputMapperTest ----------------------------------------------------------------
NameUsingOutputMapperTest::NameUsingOutputMapperTest(TString tname) : TestCaseType(tname)
{
	StartTrace(NameUsingOutputMapperTest.Ctor);
}

NameUsingOutputMapperTest::~NameUsingOutputMapperTest()
{
	StartTrace(NameUsingOutputMapperTest.Dtor);
}

//:tests if the data is put into the tempstore under the name of the Mapper
void NameUsingOutputMapperTest::NonConfiguredDestinationTest()
{
	StartTrace(NameUsingOutputMapperTest.NonConfiguredDestinationTest);

	NameUsingOutputMapper mapper("TestMapper");
	Context c;

	DoPut(mapper, c);
	c.GetTmpStore().Remove("NameUsingMapperTemp"); // remove implementation detail ??
	c.GetTmpStore().Remove("ResultMapper"); // remove implementation detail ??
	DoCheck(c.GetTmpStore());
}

//:tests if the data is put into the configured Destination
void NameUsingOutputMapperTest::ConfiguredDestinationTest()
{
	StartTrace(NameUsingOutputMapperTest.ConfiguredDestinationTest);

	NameUsingOutputMapper mapper("ConfiguredTestMapper");
	mapper.CheckConfig("ResultMapper");

	Context c;

	DoPut(mapper, c);
	DoCheck(c.GetSessionStore());

}

void NameUsingOutputMapperTest::DoPut(ResultMapper &mapper, Context &c)
{
	StartTrace(NameUsingOutputMapperTest.DoPut);

	t_assert(mapper.Put("AKey", String("AValue"), c));
	t_assert(mapper.Put("AKey", String("AnotherValue"), c));

	t_assert(mapper.Put("ALong", 1234L, c));
	t_assert(mapper.Put("ADouble", 12.34, c));

	Anything value;
	value["A"] = "ABC";
	value["B"] = 1;
	value["B"].Append(2);
	t_assert(mapper.Put("AnAny", value, c));

	String s("Streamed");
	IStringStream theStream(s);
	t_assert(mapper.Put("AStream", theStream, c));
}

void NameUsingOutputMapperTest::DoCheck(const ROAnything &result)
{
	StartTrace(NameUsingOutputMapperTest.DoCheck);

	Anything expected;
	expected["TestMapper"]["AKey"] = "AValue";
	expected["TestMapper"]["AKey"].Append("AnotherValue");
	expected["TestMapper"]["ALong"] = 1234L;
	expected["TestMapper"]["ADouble"] = 12.34;
	expected["TestMapper"]["AnAny"]["A"] = "ABC";
	expected["TestMapper"]["AnAny"]["B"] = 1;
	expected["TestMapper"]["AnAny"]["B"].Append(2);
	expected["TestMapper"]["AStream"] = "Streamed";
	assertAnyEqual(expected, result);

}

// builds up a suite of testcases, add a line for each testmethod
Test *NameUsingOutputMapperTest::suite ()
{
	StartTrace(NameUsingOutputMapperTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, NameUsingOutputMapperTest, NonConfiguredDestinationTest);
	ADD_CASE(testSuite, NameUsingOutputMapperTest, ConfiguredDestinationTest);

	return testSuite;

}
