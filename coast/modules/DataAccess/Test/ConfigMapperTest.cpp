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

//--- c-modules used -----------------------------------------------------------

//---- ConfigMapperTest ----------------------------------------------------------------
ConfigMapperTest::ConfigMapperTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(ConfigMapperTest.ConfigMapperTest);
}

TString ConfigMapperTest::getConfigFileName()
{
	return "ConfigMapperTestConfig";
}

ConfigMapperTest::~ConfigMapperTest()
{
	StartTrace(ConfigMapperTest.Dtor);
}

void ConfigMapperTest::ConfigTest()
{
	StartTrace(ConfigMapperTest.ConfigTest);

	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(caseConfig) ) {
		TString caseName;
		aEntryIterator.SlotName(caseName);
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

// builds up a suite of tests, add a line for each testmethod
Test *ConfigMapperTest::suite ()
{
	StartTrace(ConfigMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ConfigMapperTest, ConfigTest);
	return testSuite;
}
