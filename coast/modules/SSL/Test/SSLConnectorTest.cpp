/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SSLConnectorTest.h"

//--- module under test --------------------------------------------------------
#include "SSLSocket.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "SSLModule.h"
#include "Resolver.h"
#include "PoolAllocator.h"

//--- c-library modules used ---------------------------------------------------
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
	SSLConnector sslConnector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong());

	// assert the internal state of the sslConnector
	assertEqual( Resolver::DNS2IPAddress(GetConfig()["InternalSSLhost"]["ip"].AsString()), sslConnector.GetAddress() );
	assertEqual( GetConfig()["InternalSSLhost"]["port"].AsLong(), sslConnector.fPort );
	assertEqual( (long)NULL, (long)sslConnector.fSocket );

	// assert the funtionality of the public api
	Socket *socket = sslConnector.MakeSocket();
	t_assert( socket != NULL );				// these fail without HTTP Server
	t_assert( sslConnector.Use() != NULL );
	t_assert( sslConnector.GetStream() != NULL );
	delete socket;
	socket = 0;

} // simpleConstructorTest

void SSLConnectorTest::allocatorConstructorTest()
{
	{
		PoolAllocator pa(1, 8 * 1024, 21);
		TestStorageHooks tsh(&pa);

		SSLConnector connector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong(), 0L,
							   (SSL_CTX *) NULL, (const char *) NULL, 0L, true);
		Socket *socket = connector.MakeSocket();

		if ( t_assert( socket != NULL ) && t_assertm(&pa == socket->fAllocator, "allocator should match") ) {
			long socketfd = socket->GetFd();
			t_assert( socketfd > 0 );

			iostream *Ios = socket->GetStream();
			t_assert( Ios != NULL);
		}
		delete socket;
	}
	{
		TestStorageHooks tsh(Storage::Global());

		SSLConnector connector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong(), 0L,
							   (SSL_CTX *) NULL, (const char *) NULL, 0L, false);
		Socket *socket = connector.MakeSocket();

		if ( t_assert( socket != NULL ) && t_assertm(Storage::Global() == socket->fAllocator, "allocator should match") ) {
			long socketfd = socket->GetFd();
			t_assert( socketfd > 0 );

			iostream *Ios = socket->GetStream();
			t_assert( Ios != NULL);
		}
		delete socket;
	}
}

void SSLConnectorTest::ConnectAndAssert(const char *host, long port, long timeout, bool shouldFail)
{
	SSLConnector sslConnector(host, port, timeout);

	TString msg;
	msg << "with " << host << ":" << port;
	// assert the internal state of the sslConnector
	assertEqual( Resolver::DNS2IPAddress(host), sslConnector.GetAddress() );
	assertEqual( port, sslConnector.fPort );
	assertEqual( timeout, sslConnector.fConnectTimeout );
	assertEqual( (long)NULL, (long)sslConnector.fSocket );

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
	assertEqual( -1, sslConnector.fPort );

	assertEqual( (long)NULL, (long)sslConnector.MakeSocket() );
	assertEqual( (long)NULL, (long)sslConnector.Use() );
	assertEqual( (long)NULL, (long)sslConnector.GetStream() );

} // faultyConstructorTest

void SSLConnectorTest::makeSocketTest()
{
	// connect to http server on localhost
	SSLConnector sslConnector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong());

	// assert the internal state
	assertEqual( Resolver::DNS2IPAddress(GetConfig()["InternalSSLhost"]["ip"].AsString()), sslConnector.GetAddress() );
	assertEqual( GetConfig()["InternalSSLhost"]["port"].AsLong(), sslConnector.fPort );
	assertEqual( (long)NULL, (long)sslConnector.fSocket );

	// check basic functionality
	Socket *s1 = sslConnector.MakeSocket();
	Socket *s2 = sslConnector.MakeSocket();

	long sock1 = 0, sock2 = 0;

	if (s1 != 0) {
		sock1 = s1->GetFd();
		t_assert( s1 != NULL );
		if (s2 != 0) {
			t_assert( s1 != s2 );
		} // if
		delete s1;

#if defined(WIN32)
		assertEqual( -1L, ::closesocket(sock1));
#else
		assertEqual( -1L, ::close(sock1));
#endif
	}

	if (s2 != 0) {
		sock2 = s2->GetFd();

		t_assert( s2 != NULL );
		delete s2;
		// should result in an error because destructor closed socket

#if defined(WIN32)
		assertEqual( -1L, ::closesocket(sock2));
#else
		assertEqual( -1L, ::close(sock2));
#endif
	}
} // makeSocketTest

void SSLConnectorTest::makeSocketWithReuseTest()
{
	// connect to https server on localhost
	SSLConnector sslConnector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong());

	// assert the internal state
	assertEqual( Resolver::DNS2IPAddress(GetConfig()["InternalSSLhost"]["ip"].AsString()), sslConnector.GetAddress() );
	assertEqual( GetConfig()["InternalSSLhost"]["port"].AsLong(), sslConnector.fPort );
	assertEqual( (long)NULL, (long)sslConnector.fSocket );

	// check basic functionality
	Socket *s1 = sslConnector.MakeSocket(false);
	Socket *s2 = sslConnector.MakeSocket(false);

	long sock1 = 0, sock2 = 0;

	if (s1 != 0) {
		sock1 = s1->GetFd();
		t_assert( s1 != NULL );
		if (s2 != 0) {
			t_assert( s1 != s2 );
		} // if
		delete s1;

#if defined(WIN32)
		assertEqual( 0L, ::closesocket(sock1));
#else
		assertEqual( 0L, ::close(sock1));
#endif
	}

	if (s2 != 0) {
		sock2 = s2->GetFd();

		t_assert( s2 != NULL );
		delete s2;
		// should not result in an error because destructor has not closed socket
#if defined(WIN32)
		assertEqual( 0L, ::closesocket(sock2));
#else
		assertEqual( 0L, ::close(sock2));
#endif
	}
} // makeSocketTest

void SSLConnectorTest::useSocketTest()
{
	SSLConnector sslConnector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong());

	Socket *s1 = sslConnector.Use();
	Socket *s2 = sslConnector.Use();
	t_assert( s1 != NULL );			// these fail without HTTPS Server
	t_assert( s2 != NULL );
	t_assert( s1 == s2 );

	// don't delete s1 or s2
	// conncector is the owner of the
	// object
} // useSocketTest
void SSLConnectorTest::getStreamTest()
{
	// timeout 0=blocking, >0 non blocking socket
	// less than one second(1000) unreliable on marvin (solaris)
	for (long timeout = 0; timeout <= 2000; timeout += 1000) {
		SSLConnector sslConnector(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong(), timeout);
		iostream *s1 = sslConnector.GetStream();
		iostream *s2 = sslConnector.GetStream();
		TString markfailuretimeout("timeout = ");
		markfailuretimeout << timeout;
		t_assertm( s1 != NULL , markfailuretimeout);	// these fail without HTTPS Server
		t_assertm( s2 != NULL , markfailuretimeout);
		t_assert( s1 == s2 );
		Socket *sock = sslConnector.Use();
		t_assert(sock != NULL);
		if (! s1 | !sock) {
			continue;
		}

		(*s1) << "GET / HTTP/1.0" << ENDL << ENDL << flush;
		String s;
		getline(*s1, s);
		t_assert(!!(*s1));
		assertEqual( "HTTP", s.SubString(0, 4)) ;
		// test first line of reply by http server

		// don't delete s1 or s2
		// conncector is the owner of the
		// object
	}
} // getStreamTest

void SSLConnectorTest::getSessionIDTest()
{
	StartTrace(SSLConnectorTest.getSessionIDTest);

	SSLConnector sslConnector1(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong(), 1000);

	t_assert(sslConnector1.GetStream());
	Socket *sock1 = sslConnector1.Use();
	if (!t_assert(sock1)) {
		return;
	}

	TraceAny(sock1->ClientInfo(), "ClientInfo");
	t_assert(sock1->IsCertCheckPassed(Anything()));
	String session_id1(sock1->ClientInfo()["SSL"]["SessionId"].AsString());
	t_assert(session_id1.Length() > 3);
	Trace("session_id_length: " << session_id1.Length());

	SSLConnector sslConnector2(GetConfig()["InternalSSLhost"]["ip"].AsString(), GetConfig()["InternalSSLhost"]["port"].AsLong(), 1000);

	t_assert(sslConnector2.GetStream());
	Socket *sock2 = sslConnector2.Use();
	if (!t_assert(sock2)) {
		return;
	}

	TraceAny(sock2->ClientInfo(), "ClientInfo");
	t_assert(sock2->IsCertCheckPassed(Anything()));
	String session_id2(sock2->ClientInfo()["SSL"]["SessionId"].AsString());
	t_assert(session_id2.Length() > 3);
//	assertEqual(session_id1, session_id2); // session reuse does not work yet
	Trace("session_id_length: " << session_id1.Length());
}

Test *SSLConnectorTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SSLConnectorTest, simpleConstructorTest);
	ADD_CASE(testSuite, SSLConnectorTest, faultyConstructorTest);
	ADD_CASE(testSuite, SSLConnectorTest, makeSocketTest);
	ADD_CASE(testSuite, SSLConnectorTest, makeSocketWithReuseTest);
	ADD_CASE(testSuite, SSLConnectorTest, useSocketTest);
	ADD_CASE(testSuite, SSLConnectorTest, getStreamTest);
	ADD_CASE(testSuite, SSLConnectorTest, timeOutTest);
	ADD_CASE(testSuite, SSLConnectorTest, allocatorConstructorTest);
	ADD_CASE(testSuite, SSLConnectorTest, getSessionIDTest);

	return testSuite;

}
