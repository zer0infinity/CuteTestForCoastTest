/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SSLModuleTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "SSLModule.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "ROAnyLookupAdapter.h"

//--- c-modules used -----------------------------------------------------------

//---- SSLModuleTest ----------------------------------------------------------------
SSLModuleTest::SSLModuleTest(TString name) :
	ConfiguredTestCase(name, "SSLModuleTestConfig")
{
	StartTrace(SSLModuleTest.Ctor);
}

SSLModuleTest::~SSLModuleTest()
{
	StartTrace(SSLModuleTest.Dtor);
}

// setup for this TestCase
void SSLModuleTest::setUp ()
{
	StartTrace(SSLModuleTest.setUp);
	ConfiguredTestCase::setUp();
}

void SSLModuleTest::tearDown ()
{
	StartTrace(SSLModuleTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void SSLModuleTest::testCase()
{
	StartTrace(SSLModuleTest.testCase);
}

void SSLModuleTest::testLoadCertAndKey()
{
	StartTrace(SSLModuleTest.LoadCertAndKeyTest);
	FOREACH_ENTRY("LoadCertAndKeyTest", cConfig, cName) {
		Trace("At entry: " << i);
		TraceAny(cConfig, "cConfig");
		ROAnyLookupAdapter rola(cConfig["Config"]);
		if ( cConfig["TestData"]["ClientOrServer"].AsString() == "Client" ) {
			SSL_CTX *ctx = SSLModule::PrepareClientContext(&rola);
			for (long l = 0; l < cConfig["TestData"]["NumberOfRuns"].AsLong(0L); l++) {
				ctx = SSLModule::SetOwnCertificateAndKey(ctx, &rola, SSLModule::eClient);
				Trace("In iteration: " << l);
				t_assert(ctx != (SSL_CTX *) NULL);
			}
		}
		if ( cConfig["TestData"]["ClientOrServer"].AsString() == "Server" ) {
			SSL_CTX *ctx = SSLModule::PrepareServerContext(&rola);
			for (long l = 0; l < cConfig["TestData"]["NumberOfRuns"].AsLong(0L); l++) {
				ctx = SSLModule::SetOwnCertificateAndKey(ctx, &rola, SSLModule::eServer);
				Trace("In iteration: " << l);
				t_assert(ctx != (SSL_CTX *) NULL);
			}
		}
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *SSLModuleTest::suite ()
{
	StartTrace(SSLModuleTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SSLModuleTest, testCase);
	ADD_CASE(testSuite, SSLModuleTest, testLoadCertAndKey);
	return testSuite;
}
