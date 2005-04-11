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
#include "Socket.h"

//--- interface include --------------------------------------------------------
#include "ConnectorTest.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "System.h"
#include "Resolver.h"
#include "PoolAllocator.h"

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <io.h>
#endif

//---- ConnectorTest ----------------------------------------------------------------
ConnectorTest::ConnectorTest(TString tname) : TestCase(tname)
{
}

ConnectorTest::~ConnectorTest()
{
}

void ConnectorTest::setUp ()
{
	istream *is = System::OpenStream("ConnectorTest", "any");
	if ( is ) {
		fConfig.Import( *is );
		delete is;
	} else {
		assertEqual( "'read ConnectorTest.any'", "'could not read ConnectorTest.any'" );
	}
} // setUp

void ConnectorTest::tearDown ()
{
} // tearDown

void ConnectorTest::simpleConstructorTest()
{
	String theHost(fConfig["Testhost"]["ip"].AsString());
	long   thePort(fConfig["Testhost"]["port"].AsLong());
	TString msg("Connecting ");
	msg << theHost << ":" << thePort;
	Connector connector(theHost, thePort);

	// assert the internal state of the connector
	assertEqual( theHost, connector.GetAddress() );
	assertEqual( thePort, connector.fPort );
	assertEqual( (long)NULL, (long)connector.fSocket );

	// assert the funtionality of the public api
	Socket *socket = connector.MakeSocket();
	t_assertm( socket != NULL , msg);				// these fail without HTTP Server
	t_assertm( connector.Use() != NULL , msg);
	t_assertm( connector.GetStream() != NULL, msg );
	delete socket;
	socket = 0;

} // simpleConstructorTest

void ConnectorTest::ConnectAndAssert(const char *host, long port, long timeout, bool threadLocal, bool shouldFail)
{
	Connector connector(host, port, timeout, String(), 0L, threadLocal);

	// assert the internal state of the connector

	// you can not assert this anymore since in case of dns round robin load balancing
	// the ip addresses do not have to be the same e.g. www.microsoft.com
//	assertEqual( Resolver::DNS2IPAddress(host), connector.GetAddress() );
	assertEqual( port, connector.fPort );
	assertEqual( timeout, connector.fConnectTimeout );
	assertEqual( (long)NULL, (long)connector.fSocket );

	// assert the funtionality of the public api
	String assertMsg(host);
	assertMsg << ":" << port << " with timeout: " << timeout;
	String realMsg(assertMsg);
	realMsg << (( connector.Use() ) ? " has not failed" : " has failed");
	if ( shouldFail ) {
		assertMsg << " has failed";
		assertEqual(assertMsg, realMsg);
		t_assert( connector.Use() == NULL );
		t_assert( connector.GetStream() == NULL );
	} else {
		assertMsg << " has not failed";
		assertEqual(assertMsg, realMsg);
		t_assert( connector.Use() != NULL );
		t_assert( connector.GetStream() != NULL );
	}
}

void ConnectorTest::timeOutTest()
{
#if !defined(ATRAXOSA) && !defined(WIN32)
	ConnectAndAssert(fConfig["Testhost"]["ip"].AsString(), fConfig["Testhost"]["port"].AsLong(), 1L, true, false);
#else
	ConnectAndAssert(fConfig["Testhost"]["ip"].AsString(), fConfig["Testhost"]["port"].AsLong(), 1000L, true, false);
#endif
	ConnectAndAssert(fConfig["Testhost2"]["name"].AsString(), fConfig["Testhost2"]["port"].AsLong(), 5000L, true, false);
	ConnectAndAssert(fConfig["Testhost3"]["name"].AsString(), fConfig["Testhost3"]["port"].AsLong(), 100L, true, true);
} // timeOutTest

void ConnectorTest::bindingConstructorTest()
{
	ROAnything targetConfig = fConfig["Localhost"];
	const long SRC_PORT = 0L;
	const long TIMEOUT = 0L;
	// does not work on WIN32 without HTTP-server... FIXME
	Connector connector(targetConfig["ip"].AsString(), targetConfig["port"].AsLong(), TIMEOUT, targetConfig["ip"].AsString(), SRC_PORT, true);

	// assert the internal state of the connector
	assertEqual( Resolver::DNS2IPAddress(targetConfig["ip"].AsString()), connector.GetAddress() );
	assertEqual( targetConfig["port"].AsLong(), connector.fPort );
	assertEqual( (long)NULL, (long)connector.fSocket );
	assertEqual( Resolver::DNS2IPAddress(targetConfig["ip"].AsString()), connector.fSrcIPAdress );
	assertEqual( SRC_PORT, connector.fSrcPort );
	assertEqual( (long)NULL, (long)connector.fSocket );

	// assert the funtionality of the public api
	t_assert( connector.Use() != NULL );
	t_assert( connector.GetStream() != NULL );
	if (SRC_PORT > 0) {
		assertEqual( SRC_PORT, connector.GetBoundPort() );
		// it is not possible to create a second socket object with a fully bound socket
		Socket *socket = connector.MakeSocket();
		t_assert( socket == NULL );
	}
}

void ConnectorTest::faultyConstructorTest()
{
	Connector connector((const char *)NULL, -1);
	assertEqual( "", connector.GetAddress() );
	assertEqual( -1, connector.fPort );

	assertEqual( (long)NULL, (long)connector.MakeSocket() );
	assertEqual( (long)NULL, (long)connector.Use() );
	assertEqual( (long)NULL, (long)connector.GetStream() );

} // faultyConstructorTest

void ConnectorTest::makeSocketTest()
{
	// connect to http server on localhost
	Connector connector(fConfig["Testhost"]["ip"].AsString(), fConfig["Testhost"]["port"].AsLong(), 0L, String(), 0L, true);

	// assert the internal state
	assertEqual( fConfig["Testhost"]["ip"].AsString(), connector.GetAddress() );
	assertEqual( fConfig["Testhost"]["port"].AsLong(), connector.fPort );
	assertEqual( (long)NULL, (long)connector.fSocket );

	// check basic functionality
	Socket *s1 = connector.MakeSocket();
	Socket *s2 = connector.MakeSocket();

	long sock1 = 0, sock2 = 0;

	if (s1 != 0) {
		sock1 = s1->GetFd();
		t_assert( s1 != NULL );
		if (s2 != 0) {
			t_assert( s1 != s2 );
		} // if
		delete s1;
		assertEqual( -1L, ::closeSocket(sock1));
	}

	if (s2 != 0) {
		sock2 = s2->GetFd();

		t_assert( s2 != NULL );
		delete s2;
		// should result in an error because destructor closed socket
		assertEqual( -1L, ::closeSocket(sock2));
	}
} // makeSocketTest

void ConnectorTest::makeSocketWithReuseTest()
{
	// connect to http server on localhost
	Connector connector(fConfig["Testhost"]["ip"].AsString(), fConfig["Testhost"]["port"].AsLong(), 0L, String(), 0L, true);

	// assert the internal state
	assertEqual( fConfig["Testhost"]["ip"].AsString(), connector.GetAddress() );
	assertEqual( fConfig["Testhost"]["port"].AsLong(), connector.fPort );
	assertEqual( (long)NULL, (long)connector.fSocket );

	// check basic functionality
	Socket *s1 = connector.MakeSocket(false);
	Socket *s2 = connector.MakeSocket(false);

	long sock1 = 0, sock2 = 0;

	if (s1 != 0) {
		sock1 = s1->GetFd();
		t_assert( s1 != NULL );
		if (s2 != 0) {
			t_assert( s1 != s2 );
		} // if
		delete s1;
		assertEqual( 0L, ::closeSocket(sock1));
	}

	if (s2 != 0) {
		sock2 = s2->GetFd();

		t_assert( s2 != NULL );
		delete s2;
		// should not result in an error because destructor has not closed socket
		assertEqual( 0L, ::closeSocket(sock2));
	}
} // makeSocketTest

void ConnectorTest::useSocketTest()
{
	Connector connector(fConfig["Testhost"]["ip"].AsString(), fConfig["Testhost"]["port"].AsLong(), 0L, String(), 0L, true);

	Socket *s1 = connector.Use();
	Socket *s2 = connector.Use();
	t_assert( s1 != NULL );			// these fail without HTTP Server
	t_assert( s2 != NULL );
	t_assert( s1 == s2 );

	// don't delete s1 or s2
	// conncector is the owner of the
	// object
//	delete s1; // should result in SEGV
//	delete s2; // should result in SEGV
} // useSocketTest

void ConnectorTest::getStreamTest()
{
	Connector connector(fConfig["Testhost"]["ip"].AsString(), fConfig["Testhost"]["port"].AsLong(), 0L, String(), 0L, true);

	iostream *s1 = connector.GetStream();
	iostream *s2 = connector.GetStream();
	t_assert( s1 != NULL );			// these fail without HTTP Server
	t_assert( s2 != NULL );
	t_assert( s1 == s2 );

	// don't delete s1 or s2
	// conncector is the owner of the
	// object
//	delete s1; // should result in SEGV
//	delete s2; // should result in SEGV
} // useSocketTest

void ConnectorTest::allocatorConstructorTest()
{
	{
		PoolAllocator pa(1, 8 * 1024, 21);
		TestStorageHooks tsh(&pa);
		Storage::SetHooks(&tsh);
		Connector connector(fConfig["Testhost"]["ip"].AsString(), fConfig["Testhost"]["port"].AsLong(), 0L, String(), 0L, true);
		Socket *socket = connector.MakeSocket();

		if ( t_assert( socket != NULL ) && t_assertm(&pa == socket->fAllocator, "allocator should match") ) {
			long socketfd = socket->GetFd();
			t_assert( socketfd > 0 );

			iostream *Ios = socket->GetStream();
			t_assert( Ios != NULL);
		}
		delete socket;
		Storage::SetHooks(0);
	}
	{
		TestStorageHooks tsh(Storage::Global());
		Storage::SetHooks(&tsh);
		Connector connector(fConfig["Testhost"]["ip"].AsString(), fConfig["Testhost"]["port"].AsLong(), false);
		Socket *socket = connector.MakeSocket();

		if ( t_assert( socket != NULL ) && t_assertm(Storage::Global() == socket->fAllocator, "allocator should match") ) {
			long socketfd = socket->GetFd();
			t_assert( socketfd > 0 );

			iostream *Ios = socket->GetStream();
			t_assert( Ios != NULL);
		}
		delete socket;
		Storage::SetHooks(0);
	}
}

Test *ConnectorTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(ConnectorTest, simpleConstructorTest));
	testSuite->addTest (NEW_CASE(ConnectorTest, bindingConstructorTest));
	testSuite->addTest (NEW_CASE(ConnectorTest, bindingConstructorTest));
	testSuite->addTest (NEW_CASE(ConnectorTest, faultyConstructorTest));
	testSuite->addTest (NEW_CASE(ConnectorTest, makeSocketTest));
	testSuite->addTest (NEW_CASE(ConnectorTest, makeSocketWithReuseTest));
	testSuite->addTest (NEW_CASE(ConnectorTest, useSocketTest));
	testSuite->addTest (NEW_CASE(ConnectorTest, getStreamTest));
	testSuite->addTest (NEW_CASE(ConnectorTest, timeOutTest));
	testSuite->addTest (NEW_CASE(ConnectorTest, allocatorConstructorTest));

	return testSuite;

} // suite
