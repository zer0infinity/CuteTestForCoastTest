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
#include "Resolver.h"

//--- interface include --------------------------------------------------------
#include "ResolverTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "AnyIterators.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- ResolverTest ------------------------------------------------------------
ResolverTest::ResolverTest(TString tname) : TestCase(tname)
{
}

ResolverTest::~ResolverTest()
{
}

void ResolverTest::setUp ()
{
	istream *is = System::OpenStream("ResolverTest", "any");
	if ( is ) {
		fConfig.Import( *is );
		delete is;
	} else {
		assertEqual( "'read ResolverTest.any'", "'could not read ResolverTest.any'" );
	}
} // setUp

void ResolverTest::tearDown ()
{
} // tearDown

void ResolverTest::simpleDNS2IPTest()
{
	StartTrace(ResolverTest.simpleDNS2IPTest);

	AnyExtensions::Iterator<Anything> aIter(fConfig["simpleDNS2IPTest"]);
	Anything aAny;
	long lCount = 0;
	String expIP, resIP;

	while (aIter.Next(aAny)) {
		expIP = aAny["ip"].AsString();
		resIP = Resolver::DNS2IPAddress(aAny["name"].AsString());

		assertEqualm(expIP, resIP, TString("Failed at: simpleDNS2IPTest.") << fConfig["simpleDNS2IPTest"].SlotName(lCount++));
	}
}

void ResolverTest::simpleIP2DNSTest()
{
	StartTrace(ResolverTest.simpleIP2DNSTest);

	AnyExtensions::Iterator<Anything> aIter(fConfig["simpleIP2DNSTest"]);
	Anything aAny;
	long lCount = 0;
	String expDN, resDN, ip;

	while (aIter.Next(aAny)) {
		expDN = aAny["name"].AsCharPtr();
		ip = aAny["ip"].AsString();
		resDN = Resolver::IPAddress2DNS(ip);

		assertEqualm(expDN, resDN, TString("Failed at: simpleIP2DNSTest.") << fConfig["simpleIP2DNSTest"].SlotName(lCount++));
	}
}

Test *ResolverTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(ResolverTest, simpleDNS2IPTest));
	testSuite->addTest (NEW_CASE(ResolverTest, simpleIP2DNSTest));

	return testSuite;

} // suite
