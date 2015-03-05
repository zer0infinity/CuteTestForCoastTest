/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LocalizedStringsTest.h"
#include "LocalizedStrings.h"
#include "TestSuite.h"

LocalizedStringsTest::LocalizedStringsTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(LocalizedStringsTest.LocalizedStringsTest);
}

TString LocalizedStringsTest::getConfigFileName()
{
	return "LocalizedStringsTestConfig";
}

LocalizedStringsTest::~LocalizedStringsTest()
{
	StartTrace(LocalizedStringsTest.Dtor);
}

void LocalizedStringsTest::setUp ()
{
	StartTrace(LocalizedStringsTest.setUp);
	t_assert(GetConfig().IsDefined("Modules"));
	t_assert(GetConfig()["Modules"].Contains("LocalizationModule"));
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

// builds up a suite of tests, add a line for each testmethod
Test *LocalizedStringsTest::suite ()
{
	StartTrace(LocalizedStringsTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, LocalizedStringsTest, test);
	return testSuite;
}
