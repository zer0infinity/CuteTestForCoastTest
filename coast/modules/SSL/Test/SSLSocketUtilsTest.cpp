/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SSLSocketUtilsTest.h"
#include "TestSuite.h"
#include "Dbg.h"
#include "SSLSocketUtils.h"

//---- SSLSocketUtilsTest ----------------------------------------------------------------
SSLSocketUtilsTest::SSLSocketUtilsTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(SSLSocketUtilsTest.SSLSocketUtilsTest);
}

TString SSLSocketUtilsTest::getConfigFileName()
{
	return "SSLSocketUtilsTestConfig";
}

SSLSocketUtilsTest::~SSLSocketUtilsTest()
{
	StartTrace(SSLSocketUtilsTest.Dtor);
}

void SSLSocketUtilsTest::ParseDNTest()
{
	StartTrace(SSLSocketUtilsTest.ParseDNTest);
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		String dn(cConfig["DN"].AsString());
		assertAnyEqual(cConfig["Result"], SSLSocketUtils::ParseDN(dn));
	}
}

void SSLSocketUtilsTest::VerifyDNTest()
{
	StartTrace(SSLSocketUtilsTest.ParseDNTest);
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		String dnFilter(cConfig["DNFilter"].AsString());
		ROAnything dnParts(cConfig["DNParts"]);
		assertEqual(cConfig["Result"].AsBool(1), SSLSocketUtils::VerifyDN(dnFilter, dnParts));
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *SSLSocketUtilsTest::suite ()
{
	StartTrace(SSLSocketUtilsTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SSLSocketUtilsTest, ParseDNTest);
	ADD_CASE(testSuite, SSLSocketUtilsTest, VerifyDNTest);
	return testSuite;
}
