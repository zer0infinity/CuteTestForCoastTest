/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Resolver.h"
#include "TestSuite.h"
#include "ResolverTest.h"

//---- ResolverTest ------------------------------------------------------------
ResolverTest::ResolverTest(TString tname) :
	TestCaseType(tname)
{
}

ResolverTest::~ResolverTest()
{
}

void ResolverTest::simpleDNS2IPTest()
{
	StartTrace(ResolverTest.simpleDNS2IPTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(
		GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}
		String expIP, resIP;
		expIP = roaConfig["ip"].AsString();
		resIP = Resolver::DNS2IPAddress(roaConfig["name"].AsString());
		assertEqualm(expIP, resIP, TString("Failed at: ") << strCase);
	}
}

void ResolverTest::simpleIP2DNSTest()
{
	StartTrace(ResolverTest.simpleIP2DNSTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(
		GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}
		String expDN, expFQDN, resDN, ip;
		expDN = roaConfig["name"].AsString("localhost");
		expFQDN = roaConfig["fqdn"].AsString();
		ip = roaConfig["ip"].AsString();
		resDN = Resolver::IPAddress2DNS(ip);
		if ( !expDN.IsEqual(resDN) && !expFQDN.IsEqual(resDN) ) {
			assertEqualm(expDN, resDN, TString("Failed at ") << strCase);
		}
	}
}

Test *ResolverTest::suite()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ResolverTest, simpleDNS2IPTest);
	ADD_CASE(testSuite, ResolverTest, simpleIP2DNSTest);
	return testSuite;
}
