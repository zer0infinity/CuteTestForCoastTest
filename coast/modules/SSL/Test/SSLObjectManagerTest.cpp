/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SSLObjectManagerTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "SSLObjectManager.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SSLSocket.h"

//--- c-modules used -----------------------------------------------------------

//---- SSLObjectManagerTest ----------------------------------------------------------------
SSLObjectManagerTest::SSLObjectManagerTest(TString name) :
	ConfiguredTestCase(name, "SSLObjectManagerTestConfig")
{
	StartTrace(SSLObjectManagerTest.Ctor);
}

SSLObjectManagerTest::~SSLObjectManagerTest()
{
	StartTrace(SSLObjectManagerTest.Dtor);
}

// setup for this TestCase
void SSLObjectManagerTest::setUp ()
{
	StartTrace(SSLObjectManagerTest.setUp);
	ConfiguredTestCase::setUp();
}

void SSLObjectManagerTest::tearDown ()
{
	StartTrace(SSLObjectManagerTest.tearDown);
}

void SSLObjectManagerTest::testCase()
{
	StartTrace(SSLObjectManagerTest.testCase);
	ConfiguredTestCase::tearDown();
}

void SSLObjectManagerTest::GetCtxTest()
{
	StartTrace(SSLObjectManagerTest.GetCtxTest);
	FOREACH_ENTRY("GetCtxTest", cConfig, cName) {
		SSLObjectManager::SSLOBJMGR()->RemoveCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString());
		Trace("GetTest: At entry: " << i);
		TraceAny(cConfig, "cConfig");
		for (int i = 0; i < 10; i++) {
			SSL_CTX *sslctx;
			sslctx = SSLObjectManager::SSLOBJMGR()->GetCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString(), cConfig["Config"]);
			t_assert(sslctx != (SSL_CTX *) NULL);
			ConnectorArgs ca(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsLong());
			SSLSocketArgs sa(cConfig["Config"]["VerifyCertifiedEntity"].AsBool(1),
							 cConfig["Config"]["CertVerifyString"].AsString(),
							 cConfig["Config"]["CertVerifyStringIsFilter"].AsBool(1),
							 cConfig["Config"]["SessionResumption"].AsBool(0));

			SSLConnector sc(ca, sa, sslctx);
			iostream *s1 = sc.GetStream();
			Socket *s = sc.Use();
			Anything clientInfo = sc.ClientInfo();
			TraceAny(clientInfo, "clientInfo");
			assertEqual(cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0));
			assertEqual(cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0));
			assertEqual(cConfig["Results"]["IsCertCheckPassed"].AsBool(1), sc.IsCertCheckPassed(cConfig["Config"]));

			if (t_assert(s1 != NULL) && t_assert(s != NULL)) {
				TraceAny(s->ClientInfo(), "peer info");
				(*s1) << "GET / HTTP/1.0" << ENDL << ENDL << flush;
				String reply;
				getline(*s1, reply);
				t_assert(!!(*s1));
				assertEqual( "HTTP", reply.SubString(0, 4)) ;
			}
		}
	}
}

void SSLObjectManagerTest::SessionResumptionTest()
{
	StartTrace(SSLObjectManagerTest.SessionResumptionTest);

	FOREACH_ENTRY("SessionResumptionTest", cConfig, cName) {
		Trace("GetTest: At entry: " << i);
		TraceAny(cConfig, "cConfig");
		// empty  context built up by other test
		SSLObjectManager::SSLOBJMGR()->RemoveCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString());
		for (int i = 0; i < 258; i++) {
			Trace("Working at index: " << i);
			SSL_CTX *sslctx;
			sslctx = SSLObjectManager::SSLOBJMGR()->GetCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString(), cConfig["Config"]);
			t_assert(sslctx != (SSL_CTX *) NULL);
			ConnectorArgs ca(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsLong());
			SSLSocketArgs sa(cConfig["Config"]["VerifyCertifiedEntity"].AsBool(1),
							 cConfig["Config"]["CertVerifyString"].AsString(),
							 cConfig["Config"]["CertVerifyStringIsFilter"].AsBool(1),
							 cConfig["Config"]["SessionResumption"].AsBool(0));

			SSLConnector sc(ca, sa, sslctx);
			iostream *s1 = sc.GetStream();
			Socket *s = sc.Use();
			Anything clientInfo(sc.ClientInfo());
			TraceAny(clientInfo, "clientInfo");
			assertEqual(cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0));
			assertEqual(cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0));
			assertEqual(cConfig["Results"]["IsCertCheckPassed"].AsBool(1), sc.IsCertCheckPassed(cConfig["Config"]));
			if ( i == 0 || i == 256 ) {
				assertEquals(0, clientInfo["SSL"]["SessionIsResumed"].AsLong(1));
			} else {
				assertEquals(1, clientInfo["SSL"]["SessionIsResumed"].AsLong(0));
			}
			if (t_assert(s1 != NULL) && t_assert(s != NULL)) {
				(*s1) << "GET / HTTP/1.0" << ENDL << ENDL << flush;
				String reply;
				getline(*s1, reply);
				t_assert(!!(*s1));
				assertEqual( "HTTP", reply.SubString(0, 4)) ;
			}
		}
	}
}

void SSLObjectManagerTest::NoSessionResumptionTest()
{
	StartTrace(SSLObjectManagerTest.NoSessionResumptionTest);

	FOREACH_ENTRY("NoSessionResumptionTest", cConfig, cName) {
		Trace("GetTest: At entry: " << i);
		TraceAny(cConfig, "cConfig");
		// empty  context built up by other test
		SSLObjectManager::SSLOBJMGR()->RemoveCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString());
		for (int i = 0; i < 10; i++) {
			Trace("Working at index: " << i);
			SSL_CTX *sslctx;
			sslctx = SSLObjectManager::SSLOBJMGR()->GetCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString(), cConfig["Config"]);
			t_assert(sslctx != (SSL_CTX *) NULL);
			ConnectorArgs ca(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsLong());
			SSLSocketArgs sa(cConfig["Config"]["VerifyCertifiedEntity"].AsBool(1),
							 cConfig["Config"]["CertVerifyString"].AsString(),
							 cConfig["Config"]["CertVerifyStringIsFilter"].AsBool(1),
							 cConfig["Config"]["SessionResumption"].AsBool(0));

			SSLConnector sc(ca, sa, sslctx);
			iostream *s1 = sc.GetStream();
			Socket *s = sc.Use();
			Anything clientInfo(sc.ClientInfo());
			TraceAny(clientInfo, "clientInfo");
			assertEqual(cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0));
			assertEqual(cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0));
			assertEqual(cConfig["Results"]["IsCertCheckPassed"].AsBool(1), sc.IsCertCheckPassed(cConfig["Config"]));
			assertEquals(0, clientInfo["SSL"]["SessionIsResumed"].AsLong(1));
			if (t_assert(s1 != NULL) && t_assert(s != NULL)) {
				(*s1) << "GET / HTTP/1.0" << ENDL << ENDL << flush;
				String reply;
				getline(*s1, reply);
				t_assert(!!(*s1));
				assertEqual( "HTTP", reply.SubString(0, 4)) ;
			}
		}
	}
}

void SSLObjectManagerTest::GetDefaultCtxTest()
{
	StartTrace(SSLObjectManagerTest.GetDefaultCtxTest);
	FOREACH_ENTRY("GetDefaultCtxTest", cConfig, cName) {
		SSLObjectManager::SSLOBJMGR()->RemoveCtx(cConfig["Address"].AsString(), cConfig["Port"].AsString());
		Trace("GetTest: At entry: " << i);
		TraceAny(cConfig, "cConfig");
		for (int i = 0; i < 10; i++) {
			SSL_CTX *sslctx;
			sslctx = SSLObjectManager::SSLOBJMGR()->GetCtx(cConfig["Address"].AsString(), cConfig["Port"].AsString(), Anything());
			t_assert(sslctx != (SSL_CTX *) NULL);
			ConnectorArgs ca(cConfig["Address"].AsString(), cConfig["Port"].AsLong());
			SSLSocketArgs sa;

			SSLConnector sc(ca, sa, Anything());
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
		}
	}
}

void SSLObjectManagerTest::SessionIdTest()
{
	StartTrace(SSLObjectManagerTest.SessionIdTest);
	{
		FOREACH_ENTRY("SessionIdTest", cConfig, cName) {
			Trace("GetTest: At entry: " << i);
			SSL_SESSION sslSession;
			sslSession.session_id_length = sizeof(sslSession);
			long expected(cConfig["Version"].AsLong(0));
			sslSession.ssl_version = expected;
			SSLObjectManager::SSLOBJMGR()->SetSessionId(cConfig["Address"].AsString(), cConfig["Port"].AsString(), &sslSession);
			SSL_SESSION *sslSessionResult = SSLObjectManager::SSLOBJMGR()->GetSessionId(cConfig["Address"].AsString(), cConfig["Port"].AsString());
			assertEqual(sslSessionResult->ssl_version, expected);
		}
	}
	// Since we did not set real ssl sessions, freeing them in Finis() would coredump.
	SSLObjectManager::SSLOBJMGR()->EmptySessionIdStore();
}

// builds up a suite of testcases, add a line for each testmethod
Test *SSLObjectManagerTest::suite ()
{
	StartTrace(SSLObjectManagerTest.suite);
	TestSuite *testSuite = new TestSuite;

//	ADD_CASE(testSuite, SSLObjectManagerTest, testCase);
	ADD_CASE(testSuite, SSLObjectManagerTest, GetCtxTest);
	ADD_CASE(testSuite, SSLObjectManagerTest, GetDefaultCtxTest);
	ADD_CASE(testSuite, SSLObjectManagerTest, SessionIdTest);
	ADD_CASE(testSuite, SSLObjectManagerTest, SessionResumptionTest);
	ADD_CASE(testSuite, SSLObjectManagerTest, NoSessionResumptionTest);
	return testSuite;
}
