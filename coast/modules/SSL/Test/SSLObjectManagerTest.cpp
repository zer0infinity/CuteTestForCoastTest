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

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//--- c-modules used -----------------------------------------------------------

//---- SSLObjectManagerTest ----------------------------------------------------------------
SSLObjectManagerTest::SSLObjectManagerTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(SSLObjectManagerTest.SSLObjectManagerTest);
}

TString SSLObjectManagerTest::getConfigFileName()
{
	return "SSLObjectManagerTestConfig";
}

SSLObjectManagerTest::~SSLObjectManagerTest()
{
	StartTrace(SSLObjectManagerTest.Dtor);
}

void SSLObjectManagerTest::UsePassedInCtxTest()
{
	StartTrace(SSLObjectManagerTest.UsePassedInCtxTest);
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		SSLObjectManager::SSLOBJMGR()->RemoveCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString());
		TraceAny(cConfig, "cConfig");
		for (int ii = 0; ii < 10; ii++) {
			Trace("At run index: " << ii);
			SSL_CTX *sslctx;
			sslctx = SSLObjectManager::SSLOBJMGR()->GetCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString(), cConfig["Config"]);
			t_assert(sslctx != (SSL_CTX *) NULL);
			ConnectorArgs ca(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsLong());
			SSLSocketArgs sa(cConfig["Config"]["VerifyCertifiedEntity"].AsBool(1),
							 cConfig["Config"]["CertVerifyString"].AsString(),
							 cConfig["Config"]["CertVerifyStringIsFilter"].AsBool(1),
							 cConfig["Config"]["SessionResumption"].AsBool(0));

			SSLConnector sc(ca, sa, Anything(), sslctx, (const char *) NULL, 0L, cConfig["Config"]["UseThreadLocalMemory"].AsLong(0L));
			iostream *s1 = sc.GetStream();
			Socket *s = sc.Use();
			Anything clientInfo = sc.ClientInfo();
			TraceAny(clientInfo, "clientInfo");
			assertEqual(cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0));
			assertEqual(cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0));
			assertEqual(cConfig["Results"]["IsCertCheckPassed"].AsBool(1), s->IsCertCheckPassed(cConfig["Config"]));

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

void SSLObjectManagerTest::ReUseCreatedCtxTest()
{
	StartTrace(SSLObjectManagerTest.ReUseCreatedCtxTest);
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		SSLObjectManager::SSLOBJMGR()->RemoveCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString());
		TraceAny(cConfig, "cConfig");
		for (int ii = 0; ii < 10; ii++) {
			Trace("At run index: " << ii);
			SSL_CTX *sslctx;
			sslctx = SSLObjectManager::SSLOBJMGR()->GetCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString(), cConfig["Config"]);
			t_assert(sslctx != (SSL_CTX *) NULL);
			ConnectorArgs ca(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsLong());
			SSLSocketArgs sa(cConfig["Config"]["VerifyCertifiedEntity"].AsBool(1),
							 cConfig["Config"]["CertVerifyString"].AsString(),
							 cConfig["Config"]["CertVerifyStringIsFilter"].AsBool(1),
							 cConfig["Config"]["SessionResumption"].AsBool(0));

			SSLConnector sc(ca, sa, Anything(), NULL, (const char *) NULL, 0L, cConfig["Config"]["UseThreadLocalMemory"].AsLong(0L));
			iostream *s1 = sc.GetStream();
			Socket *s = sc.Use();
			Anything clientInfo = sc.ClientInfo();
			TraceAny(clientInfo, "clientInfo");
			assertEqual(cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0));
			assertEqual(cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0));
			assertEqual(cConfig["Results"]["IsCertCheckPassed"].AsBool(1), s->IsCertCheckPassed(cConfig["Config"]));

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

void SSLObjectManagerTest::SessionResumptionWithMinimumConfigTest()
{
	StartTrace(SSLObjectManagerTest.SessionResumptionWithMinimumConfigTest);

	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		TraceAny(cConfig, "cConfig");
		// empty  context built up by other test
		SSLObjectManager::SSLOBJMGR()->RemoveCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString());
		for (int ii = 0; ii < 258; ii++) {
			Trace("At run index: " << ii);
			SSLConnector sc(cConfig["Config"]);
			iostream *s1 = sc.GetStream();
			Socket *s = sc.Use();
			Anything clientInfo(sc.ClientInfo());
			TraceAny(clientInfo, "clientInfo");
			assertEqual(cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0));
			assertEqual(cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0));
			assertEqual(cConfig["Results"]["IsCertCheckPassed"].AsBool(1), s->IsCertCheckPassed(cConfig["Config"]));
			if ( ii == 0 || ii == 256 ) {
				assertEqual(0, clientInfo["SSL"]["SessionIsResumed"].AsLong(1));
			} else {
				assertEqual(1, clientInfo["SSL"]["SessionIsResumed"].AsLong(0));
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

void SSLObjectManagerTest::SessionResumptionTest()
{
	StartTrace(SSLObjectManagerTest.SessionResumptionTest);

	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		TraceAny(cConfig, "cConfig");
		// empty  context built up by other test
		SSLObjectManager::SSLOBJMGR()->RemoveCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString());
		for (int ii = 0; ii < 258; ii++) {
			Trace("At run index: " << ii);
			SSL_CTX *sslctx;
			sslctx = SSLObjectManager::SSLOBJMGR()->GetCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString(), cConfig["Config"]);
			t_assert(sslctx != (SSL_CTX *) NULL);
			ConnectorArgs ca(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsLong());
			SSLSocketArgs sa(cConfig["Config"]["VerifyCertifiedEntity"].AsBool(1),
							 cConfig["Config"]["CertVerifyString"].AsString(),
							 cConfig["Config"]["CertVerifyStringIsFilter"].AsBool(1),
							 cConfig["Config"]["SessionResumption"].AsBool(0));

			SSLConnector sc(ca, sa, Anything(), sslctx, (const char *) NULL, 0L, cConfig["Config"]["UseThreadLocalMemory"].AsLong(0L));
			iostream *s1 = sc.GetStream();
			Socket *s = sc.Use();
			Anything clientInfo(sc.ClientInfo());
			TraceAny(clientInfo, "clientInfo");
			assertEqual(cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0));
			assertEqual(cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0));
			assertEqual(cConfig["Results"]["IsCertCheckPassed"].AsBool(1), s->IsCertCheckPassed(cConfig["Config"]));
			if ( ii == 0 || ii == 256 ) {
				assertEqual(0, clientInfo["SSL"]["SessionIsResumed"].AsLong(1));
			} else {
				assertEqual(1, clientInfo["SSL"]["SessionIsResumed"].AsLong(0));
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

	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		TraceAny(cConfig, "cConfig");
		// empty  context built up by other test
		SSLObjectManager::SSLOBJMGR()->RemoveCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString());
		for (int ii = 0; ii < 10; ii++) {
			Trace("At run index: " << ii);
			SSL_CTX *sslctx;
			sslctx = SSLObjectManager::SSLOBJMGR()->GetCtx(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsString(), cConfig["Config"]);
			t_assert(sslctx != (SSL_CTX *) NULL);
			ConnectorArgs ca(cConfig["Config"]["Address"].AsString(), cConfig["Config"]["Port"].AsLong());
			SSLSocketArgs sa(cConfig["Config"]["VerifyCertifiedEntity"].AsBool(1),
							 cConfig["Config"]["CertVerifyString"].AsString(),
							 cConfig["Config"]["CertVerifyStringIsFilter"].AsBool(1),
							 cConfig["Config"]["SessionResumption"].AsBool(0));

			SSLConnector sc(ca, sa, Anything(), sslctx, (const char *) NULL, 0L, cConfig["Config"]["UseThreadLocalMemory"].AsLong(0L));
			iostream *s1 = sc.GetStream();
			Socket *s = sc.Use();
			Anything clientInfo(sc.ClientInfo());
			TraceAny(clientInfo, "clientInfo");
			assertEqual(cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0));
			assertEqual(cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1), clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0));
			assertEqual(cConfig["Results"]["IsCertCheckPassed"].AsBool(1), s->IsCertCheckPassed(cConfig["Config"]));
			assertEqual(0, clientInfo["SSL"]["SessionIsResumed"].AsLong(1));
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
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		SSLObjectManager::SSLOBJMGR()->RemoveCtx(cConfig["Address"].AsString(), cConfig["Port"].AsString());
		TraceAny(cConfig, "cConfig");
		for (int ii = 0; ii < 10; ii++) {
			Trace("At run index: " << ii);
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
		ROAnything cConfig;
		AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
		while ( aEntryIterator.Next(cConfig) ) {
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
	ADD_CASE(testSuite, SSLObjectManagerTest, UsePassedInCtxTest);
	ADD_CASE(testSuite, SSLObjectManagerTest, ReUseCreatedCtxTest);
	ADD_CASE(testSuite, SSLObjectManagerTest, GetDefaultCtxTest);
	ADD_CASE(testSuite, SSLObjectManagerTest, SessionIdTest);
	ADD_CASE(testSuite, SSLObjectManagerTest, SessionResumptionTest);
	ADD_CASE(testSuite, SSLObjectManagerTest, SessionResumptionWithMinimumConfigTest);
	ADD_CASE(testSuite, SSLObjectManagerTest, NoSessionResumptionTest);
	return testSuite;
}
