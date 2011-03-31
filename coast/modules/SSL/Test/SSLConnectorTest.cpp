/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SSLConnectorTest.h"
#include "SSLSocket.h"
#include "TestSuite.h"
#include "SSLModule.h"
#include "Resolver.h"
#include "PoolAllocator.h"
#if defined(WIN32)
#include <io.h>
#endif

//---- SSLConnectorTest ----------------------------------------------------------------
SSLConnectorTest::SSLConnectorTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(SSLConnectorTest.SSLConnectorTest);
}

SSLConnectorTest::~SSLConnectorTest()
{
}

void SSLConnectorTest::setUp ()
{
	StartTrace(SSLConnectorTest.setUp);
	WDModule *sslmodule = WDModule::FindWDModule("SSLModule");
	sslmodule->ResetInit(Anything());
}

void SSLConnectorTest::tearDown ()
{
	StartTrace(SSLConnectorTest.tearDown);
	WDModule *sslmodule = WDModule::FindWDModule("SSLModule");
	sslmodule->ResetFinis(Anything());
}

void SSLConnectorTest::simpleConstructorTest()
{
	StartTrace(SSLConnectorTest.simpleConstructorTest);
	SSLConnector sslConnector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong());

	// assert the internal state of the sslConnector
	assertEqual( Resolver::DNS2IPAddress(GetConfig()["InternalSSLhost"]["ip"].AsString()), sslConnector.GetAddress() );
	assertEqual( GetConfig()["InternalSSLhost"]["port"].AsLong(), sslConnector.GetPort() );

	// assert the functionality of the public api
	t_assert( sslConnector.Use() != NULL );
	t_assert( sslConnector.GetStream() != NULL );
} // simpleConstructorTest

void SSLConnectorTest::allocatorConstructorTest()
{
	StartTrace(SSLConnectorTest.allocatorConstructorTest);
	{
		PoolAllocator pa(1, 8 * 1024, 21);
		TestStorageHooks tsh(&pa);

		SSLConnector connector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong(), 0L,
				(SSL_CTX *) NULL, (const char *) NULL, 0L, true);
		Socket *socket = connector.Use();
		if (t_assert(socket != NULL) && t_assertm(&pa == socket->GetAllocator(), "allocator should match")) {
			long socketfd = socket->GetFd();
			t_assert(socketfd > 0);
			std::iostream *Ios = socket->GetStream();
			t_assert(Ios != NULL);
		}
	}
	{
		TestStorageHooks tsh(Coast::Storage::Global());
		SSLConnector connector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong(), 0L,
				(SSL_CTX *) NULL, (const char *) NULL, 0L, false);
		Socket *socket = connector.Use();

		if (t_assert(socket != NULL) && t_assertm(Coast::Storage::Global() == socket->GetAllocator(), "allocator should match")) {
			long socketfd = socket->GetFd();
			t_assert(socketfd > 0);
			std::iostream *Ios = socket->GetStream();
			t_assert(Ios != NULL);
		}
	}
}

void SSLConnectorTest::ConnectAndAssert(const char *host, long port, long timeout, bool shouldFail)
{
	SSLConnector sslConnector(host, port, timeout);

	TString msg;
	msg << "with " << host << ":" << port;
	// assert the internal state of the sslConnector
	assertEqual( Resolver::DNS2IPAddress(host), sslConnector.GetAddress() );
	assertEqual( port, sslConnector.GetPort() );
	assertEqual( timeout, sslConnector.GetTimeout() );

	// assert the funtionality of the public api
	String assertMsg(host);
	assertMsg << ":" << port << " with timeout: " << timeout;
	String realMsg(assertMsg);
	realMsg << (( sslConnector.Use() ) ? " has not failed" : " has failed");
	if ( shouldFail ) {
		assertMsg << " has failed";
		assertEqual(assertMsg, realMsg);
		t_assertm(NULL == sslConnector.Use(), msg);
		t_assertm(NULL == sslConnector.GetStream(), msg);
	} else {
		assertMsg << " has not failed";
		assertEqual(assertMsg, realMsg);
		t_assertm( sslConnector.Use() != NULL , msg);
		t_assertm( sslConnector.GetStream() != NULL , msg);
	}
}

void SSLConnectorTest::timeOutTest()
{
	ConnectAndAssert(GetConfig()["RemoteSSLhost"]["name"].AsString(), GetConfig()["RemoteSSLhost"]["port"].AsLong(), 1000L, false);
	ConnectAndAssert(GetConfig()["RemoteSSLhost"]["name"].AsString(), GetConfig()["RemoteSSLhost"]["faultyport"].AsLong(), 100L, true);
} // timeOutTest

void SSLConnectorTest::faultyConstructorTest()
{
	SSLConnector sslConnector(NULL, -1);
	assertEqual( "", sslConnector.GetAddress() );
	assertEqual( -1, sslConnector.GetPort() );

	assertEqual( (long)NULL, (long)sslConnector.Use() );
	assertEqual( (long)NULL, (long)sslConnector.GetStream() );
} // faultyConstructorTest

void SSLConnectorTest::getStreamTest()
{
	// timeout 0=blocking, >0 non blocking socket
	// less than one second(1000) unreliable on marvin (solaris)
	for (long timeout = 0; timeout <= 2000; timeout += 1000) {
		SSLConnector sslConnector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong(), timeout);
		std::iostream *s1 = sslConnector.GetStream();
		TString markfailuretimeout("timeout = ");
		markfailuretimeout << timeout;
		t_assertm( s1 != NULL , markfailuretimeout);	// these fail without HTTPS Server
		if (!s1) {
			continue;
		}
		(*s1) << "GET / HTTP/1.0" << ENDL << ENDL << std::flush;
		String s;
		getline(*s1, s);
		t_assert(!!(*s1));
		assertEqual( "HTTP", s.SubString(0, 4)) ;
	}
} // getStreamTest

void SSLConnectorTest::getSessionIDTest()
{
	StartTrace(SSLConnectorTest.getSessionIDTest);

	String session_id1, session_id2;
	{
		SSLConnector sslConnector1(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong(), 1000);

		t_assert(sslConnector1.GetStream());
		Socket *sock1 = sslConnector1.Use();
		if (!t_assert(sock1)) {
			return;
		}

		TraceAny(sock1->ClientInfo(), "ClientInfo");
		t_assert(sock1->IsCertCheckPassed(Anything()));
		session_id1 = sock1->ClientInfo()["SSL"]["SessionId"].AsString();
		t_assert(session_id1.Length() > 3);
		Trace("session_id1: " << session_id1);
	}
	{
		SSLConnector sslConnector2(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong(), 1000);

		t_assert(sslConnector2.GetStream());
		Socket *sock2 = sslConnector2.Use();
		if (!t_assert(sock2)) {
			return;
		}

		TraceAny(sock2->ClientInfo(), "ClientInfo");
		t_assert(sock2->IsCertCheckPassed(Anything()));
		session_id2 = sock2->ClientInfo()["SSL"]["SessionId"].AsString();
		t_assert(session_id2.Length() > 3);
		Trace("session_id2: " << session_id2);
	}
//	assertEqual(session_id1, session_id2); // session reuse does not work yet
}

Test *SSLConnectorTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SSLConnectorTest, simpleConstructorTest);
	ADD_CASE(testSuite, SSLConnectorTest, faultyConstructorTest);
	ADD_CASE(testSuite, SSLConnectorTest, getStreamTest);
	ADD_CASE(testSuite, SSLConnectorTest, timeOutTest);
	ADD_CASE(testSuite, SSLConnectorTest, allocatorConstructorTest);
	ADD_CASE(testSuite, SSLConnectorTest, getSessionIDTest);
	return testSuite;
}
