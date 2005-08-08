/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ConfiguredLookupAdapterTest.h"

//--- module under test --------------------------------------------------------
#include "ConfiguredLookupAdapter.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- ConfiguredLookupAdapterTest ----------------------------------------------------------------
ConfiguredLookupAdapterTest::ConfiguredLookupAdapterTest(TString tstrName)
	: ConfiguredTestCase(tstrName, "ConfiguredLookupAdapterTestConfig")
{
	StartTrace(ConfiguredLookupAdapterTest.Ctor);
}

ConfiguredLookupAdapterTest::~ConfiguredLookupAdapterTest()
{
	StartTrace(ConfiguredLookupAdapterTest.Dtor);
}

// setup for this ConfiguredTestCase
void ConfiguredLookupAdapterTest::setUp ()
{
	StartTrace(ConfiguredLookupAdapterTest.setUp);
	ConfiguredTestCase::setUp();
}

void ConfiguredLookupAdapterTest::tearDown ()
{
	StartTrace(ConfiguredLookupAdapterTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void ConfiguredLookupAdapterTest::testConfiguredLookupAdapter()
{
	StartTrace(ConfiguredLookupAdapterTest.test);
	FOREACH_ENTRY("TestConfiguredLookupAdapter", caseConfig, testName) {
		ROAnything conf(caseConfig["Config"]);
		ROAnything def(caseConfig["Default"]);

		ConfiguredLookupAdapter cla(conf, def);
		assertEquals(caseConfig["ExpectedString"].AsString(), cla.Lookup(caseConfig["LookupPathString"].AsString(), ""));
		assertEquals(caseConfig["ExpectedLong"].AsLong(1L), cla.Lookup(caseConfig["LookupPathLong"].AsString(""), 0L));
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *ConfiguredLookupAdapterTest::suite ()
{
	StartTrace(ConfiguredLookupAdapterTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ConfiguredLookupAdapterTest, testConfiguredLookupAdapter);

	return testSuite;
}
