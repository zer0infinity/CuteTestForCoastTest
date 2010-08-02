/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "DataAccessImplTest.h"

//--- module under test --------------------------------------------------------
#include "DataAccessImpl.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Context.h"

//---- DataAccessImplTest ----------------------------------------------------------------
Test *DataAccessImplTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, DataAccessImplTest, GetConfigNameTest);
	ADD_CASE(testSuite, DataAccessImplTest, DoLoadConfigTest);
	return testSuite;
}

DataAccessImplTest::DataAccessImplTest(TString tname) :
	TestCaseType(tname) {
}

DataAccessImplTest::~DataAccessImplTest() {
}

void DataAccessImplTest::GetConfigNameTest() {
	DataAccessImpl dai("testdai");
	assertEqual("DataAccessImplMeta", dai.GetConfigName());
}

void DataAccessImplTest::DoLoadConfigTest() {
	DataAccessImpl dai("testdai");
	t_assert( dai.Initialize("DataAccessImpl") );
	t_assert(dai.GetConfig().IsDefined("testitem1"));
	assertEqual("foo", dai.GetConfig()["testitem1"].AsCharPtr());
	t_assert(dai.GetConfig().IsDefined("testitem2"));
	assertEqual("bah", dai.GetConfig()["testitem2"].AsCharPtr());
}
