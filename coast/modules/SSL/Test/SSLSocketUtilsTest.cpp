/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SSLSocketUtilsTest.h"

//--- module under test --------------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SSLSocketUtils.h"

//--- c-modules used -----------------------------------------------------------

//---- SSLSocketUtilsTest ----------------------------------------------------------------
SSLSocketUtilsTest::SSLSocketUtilsTest(TString name)
	: ConfiguredTestCase(name, "SSLSocketUtilsTestConfig")
{
	StartTrace(SSLSocketUtilsTest.Ctor);
}

SSLSocketUtilsTest::~SSLSocketUtilsTest()
{
	StartTrace(SSLSocketUtilsTest.Dtor);
}

// setup for this ConfiguredTestCase
void SSLSocketUtilsTest::setUp ()
{
	StartTrace(SSLSocketUtilsTest.setUp);
	ConfiguredTestCase::setUp();
}

void SSLSocketUtilsTest::tearDown ()
{
	StartTrace(SSLSocketUtilsTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void SSLSocketUtilsTest::test()
{
	StartTrace(SSLSocketUtilsTest.test);
}

void SSLSocketUtilsTest::ParseDNTest()
{
	StartTrace(SSLSocketUtilsTest.ParseDNTest);
	FOREACH_ENTRY("ParseDNTest", cConfig, cName) {
		Trace("ParseDNTest: At entry: " << i);
		String dn(cConfig["DN"].AsString());
		assertAnyEqual(cConfig["Result"], SSLSocketUtils::ParseDN(dn));
	}
}

void SSLSocketUtilsTest::VerifyDNTest()
{
	StartTrace(SSLSocketUtilsTest.ParseDNTest);
	FOREACH_ENTRY("VerifyDNTest", cConfig, cName) {
		Trace("VerifyDNTest: At entry: " << i);
		String dnFilter(cConfig["DNFilter"].AsString());
		ROAnything dnParts(cConfig["DNParts"]);
		assertEqual(cConfig["Result"].AsBool(1), SSLSocketUtils::VerifyDN(dnFilter, dnParts));
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *SSLSocketUtilsTest::suite ()
{
	StartTrace(SSLSocketUtilsTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SSLSocketUtilsTest, test);
	ADD_CASE(testSuite, SSLSocketUtilsTest, ParseDNTest);
	ADD_CASE(testSuite, SSLSocketUtilsTest, VerifyDNTest);
	return testSuite;
}
