/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "DataAccessImpl.h"

//--- interface include --------------------------------------------------------
#include "DataAccessImplTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Context.h"

//--- c-library modules used ---------------------------------------------------

//---- DataAccessImplTest ----------------------------------------------------------------
Test *DataAccessImplTest::suite ()
// collect all test cases for the RegistryStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(DataAccessImplTest, GetConfigNameTest));
	testSuite->addTest (NEW_CASE(DataAccessImplTest, DoLoadConfigTest));
	testSuite->addTest (NEW_CASE(DataAccessImplTest, GetInputMapperTest));
	testSuite->addTest (NEW_CASE(DataAccessImplTest, GetOutputMapperTest));

	return testSuite;

} // suite

DataAccessImplTest::DataAccessImplTest(TString tname) : TestCase(tname)
{

}

DataAccessImplTest::~DataAccessImplTest()
{

}

void DataAccessImplTest::setUp ()
// setup connector for this TestCase
{

} // setUp

void DataAccessImplTest::GetConfigNameTest()
{
	String configName;
	DataAccessImpl dai("testdai");

	t_assert( dai.DoGetConfigName("DataAccessImpl", "testdai", configName) );
	assertEqual("DataAccessImplMeta", configName);

}

void DataAccessImplTest::DoLoadConfigTest()
{
	DataAccessImpl dai("testdai");

	t_assert( dai.DoLoadConfig("DataAccessImpl") );
	t_assert(dai.fConfig.IsDefined("testitem1"));
	assertEqual("foo", dai.fConfig["testitem1"].AsCharPtr());
	t_assert(dai.fConfig.IsDefined("testitem2"));
	assertEqual("bah", dai.fConfig["testitem2"].AsCharPtr());
}

void DataAccessImplTest::GetInputMapperTest()
{
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	Anything tmpStore(ctx.GetTmpStore());

	const char *daName = "daTest";
	DataAccessImpl daTest(daName);
	InputMapper *m = daTest.GetInputMapper( ctx);

	t_assert( m != 0 );

	daName = "daTestNone";
	DataAccessImpl daTestNone(daName);
	InputMapper *none = daTestNone.GetInputMapper( ctx);

	t_assert( none == 0 );
	String str(tmpStore["DataAccess"][daName]["Error"][0L].AsCharPtr());
	t_assertm(str.Contains("Mapper::FindInputMapper returned 0 for daTestNone") >= 0, "Mapper::FindInputMapper errormsg not OK");

}

void DataAccessImplTest::GetOutputMapperTest()
{
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	Anything tmpStore(ctx.GetTmpStore());

	const char *daName = "daTest";
	DataAccessImpl daTest(daName);
	OutputMapper *m = daTest.GetOutputMapper( ctx);

	t_assert( m != 0 );

	daName = "daTestNone";
	DataAccessImpl daTestNone(daName);
	OutputMapper *none = daTestNone.GetOutputMapper( ctx);

	t_assert( none == 0 );
	String str(tmpStore["DataAccess"][daName]["Error"][0L].AsCharPtr());
	t_assertm(str.Contains("Mapper::FindOutputMapper returned 0 for daTestNone") >= 0, "Mapper::FindOutputMapper errormsg not OK");

}

