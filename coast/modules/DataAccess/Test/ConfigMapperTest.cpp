/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ConfigMapperTest.h"

//--- module under test --------------------------------------------------------
#include "Mapper.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- ConfigMapperTest ----------------------------------------------------------------
ConfigMapperTest::ConfigMapperTest(TString tstrName)
	: ConfiguredTestCase(tstrName, "ConfigMapperTestConfig")
{
	StartTrace(ConfigMapperTest.Ctor);
}

ConfigMapperTest::~ConfigMapperTest()
{
	StartTrace(ConfigMapperTest.Dtor);
}

// setup for this ConfiguredTestCase
void ConfigMapperTest::setUp ()
{
	StartTrace(ConfigMapperTest.setUp);
	ConfiguredTestCase::setUp();
}

void ConfigMapperTest::tearDown ()
{
	StartTrace(ConfigMapperTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void ConfigMapperTest::testConfigMapper()
{
	StartTrace(ConfigMapperTest.testConfigMapper);

	FOREACH_ENTRY("ConfigMapperTest", caseConfig, caseName) {
		Trace("Running " << caseName << " Test");

		// prepare tmp store
		Context ctx;
		ctx.GetTmpStore() = caseConfig["TmpStore"].DeepClone();

		// call
		Anything result;
		ConfigMapper cm("Test");
		cm.DoGetAny("", result, ctx, caseConfig["MapperConfig"]);
		TraceAny(result, "resulting config: ");

		// compare result and expected
		TraceAny(ctx.GetTmpStore(), "tmp store after:");
		assertAnyEqual(caseConfig["Expected"], result);
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *ConfigMapperTest::suite ()
{
	StartTrace(ConfigMapperTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ConfigMapperTest, testConfigMapper);

	return testSuite;
}
