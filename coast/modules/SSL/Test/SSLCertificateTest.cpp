/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SSLCertificateTest.h"

//--- module under test --------------------------------------------------------
#include "SSLSocket.h"
//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- SSLCertificateTest ----------------------------------------------------------------
SSLCertificateTest::SSLCertificateTest(TString tstrName)
	: ConfiguredTestCase(tstrName, "SSLCertificateTest")
{
	StartTrace(SSLCertificateTest.Ctor);
}

SSLCertificateTest::~SSLCertificateTest()
{
	StartTrace(SSLCertificateTest.Dtor);
}

// setup for this ConfiguredTestCase
void SSLCertificateTest::setUp ()
{
	StartTrace(SSLCertificateTest.setUp);
	ConfiguredTestCase::setUp();
}

void SSLCertificateTest::tearDown ()
{
	StartTrace(SSLCertificateTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void SSLCertificateTest::test()
{
	t_assert(false);
}

void SSLCertificateTest::clientCertificateTest()
{
	StartTrace(SSLCertificateTest.clientCertificateTest);
	FOREACH_ENTRY("ClientCertificateTest", cConfig, cName) {
		Trace("ClientCertificateTest: At entry: " << i);
		SSLConnector sc(cConfig["Config"], (SSL_CTX *) NULL); // add a new connector type using a config
		iostream *s1 = sc.GetStream();
		Socket *s = sc.Use();
		Anything clientInfo(sc.ClientInfo());
		assertEqual(cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0));
		assertEqual(cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0));
		assertEqual(cConfig["Results"]["IsCertCheckPassed"].AsBool(1), sc.IsCertCheckPassed(cConfig["Config"]));
		if ( cConfig["Results"]["GetRequestOk"].AsBool(1) ) {
			if (t_assert(s1 != NULL) && t_assert(s != NULL)) {
				TraceAny(s->ClientInfo(), "peer info");
				(*s1) << "GET / HTTP/1.0" << ENDL << ENDL << flush;
				String reply;
				getline(*s1, reply);
				assertEqual(cConfig["Results"]["GetRequestOk"].AsBool(1) , !!(*s1));
				assertEqual( "HTTP", reply.SubString(0, 4)) ;
			}
		}
	}

//	t_assert(false);
	//check
} // getStreamTest
void SSLCertificateTest::checkServerCertificateTest()
{
	StartTrace(SSLCertificateTest.checkServerCertificateTest);
	ROAnything scfg(fConfig["RemoteCertificateHost"]);
	SSLConnector sc(scfg, (SSL_CTX *) NULL); // add a new connector type using a config
	iostream *s1 = sc.GetStream();
	Socket *s = sc.Use();
	if (t_assert(s1 != NULL) && t_assert(s != NULL)) {
		TraceAny(s->ClientInfo(), "peer info");
		(*s1) << "GET / HTTP/1.0" << ENDL << ENDL << flush;
		String reply;
		getline(*s1, reply);
		t_assert(!!(*s1));
		assertEqual( "HTTP", reply.SubString(0, 4)) ;
	}
	//check server cert
} // getStreamTest

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *SSLCertificateTest::suite ()
{
	StartTrace(SSLCertificateTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SSLCertificateTest, clientCertificateTest);
	ADD_CASE(testSuite, SSLCertificateTest, checkServerCertificateTest);

	return testSuite;
}
