/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "WDModule.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "LocalizedStrings.h"

//--- interface include --------------------------------------------------------
#include "LocalizedStringsTest.h"

//---- LocalizedStringsTest ----------------------------------------------------------------
LocalizedStringsTest::LocalizedStringsTest(TString name)
	: ConfiguredTestCase(name, "LocalizedStringsTestConfig")
{
	StartTrace(LocalizedStringsTest.Ctor);
}

LocalizedStringsTest::~LocalizedStringsTest()
{
	StartTrace(LocalizedStringsTest.Dtor);
}

// setup for this ConfiguredTestCase
void LocalizedStringsTest::setUp ()
{
	StartTrace(LocalizedStringsTest.setUp);
	ConfiguredTestCase::setUp();

	t_assert(fConfig.IsDefined("Modules"));
	t_assert(fConfig["Modules"].Contains("LocalizationModule"));
	WDModule::Install(fConfig);
}

void LocalizedStringsTest::tearDown ()
{
	StartTrace(LocalizedStringsTest.tearDown);

	WDModule::Terminate(fConfig);
	ConfiguredTestCase::tearDown();
}

void LocalizedStringsTest::test()
{
	StartTrace(LocalizedStringsTest.test);
	LocalizedStrings *ls = LocalizedStrings::LocStr();
	ROAnything result;
	if (t_assert(ls != NULL)) {
		t_assert(ls->Lookup("Modules", result));

		t_assert(ls->Lookup("News_L", result));
		TraceAny(result, "result");;
		assertEqual("Nachrichten", result["String"]["D"].AsCharPtr());
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *LocalizedStringsTest::suite ()
{
	StartTrace(LocalizedStringsTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(LocalizedStringsTest, test));

	return testSuite;
} // suite
