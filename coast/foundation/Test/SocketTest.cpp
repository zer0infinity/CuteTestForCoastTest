/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SocketTest.h"

//--- module under test --------------------------------------------------------
#include "Socket.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "StringStream.h"
#include "PoolAllocator.h"

//--- c-library modules used ---------------------------------------------------
#if !defined(WIN32)
#if !defined(__370__)
#include <netinet/in.h>		// for INADDR_ANY
#endif
#include <fcntl.h>
#endif

//---- SocketTest ------------------------------------------------------------
SocketTest::SocketTest(TString tname)
	: TestCaseType(tname)
{
}

SocketTest::~SocketTest()
{
}

void SocketTest::simpleConstructorTest()
{
	Connector connector(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port"].AsLong());
	Socket *socket = connector.MakeSocket();

	if ( t_assert( socket != NULL ) ) { // fails without HTTP Server
		long socketfd = socket->GetFd();
		t_assert( socketfd > 0 );

		iostream *Ios = socket->GetStream();
		t_assert( Ios != NULL);
	}
	delete socket;
} // simpleConstructorTest

void SocketTest::allocatorConstructorTest()
{
	PoolAllocator pa(1, 8 * 1024, 21);
	TestStorageHooks tsh(&pa);

	Connector connector(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port"].AsLong());
	connector.SetThreadLocal();
	Socket *socket = connector.MakeSocket();

	if ( t_assert( socket != NULL ) ) { // fails without HTTP Server
		long socketfd = socket->GetFd();
		t_assert( socketfd > 0 );

		iostream *Ios = socket->GetStream();
		t_assert( Ios != NULL);
	}
	delete socket;
}

void SocketTest::faultyConstructorTest()
{
	Socket socket1(-1);
	long socketfd = socket1.GetFd();

	assertEqual(-1, socketfd);

	iostream *Ios = socket1.GetStream();
	assertEqual( (long)NULL, (long)Ios );

	assertEqual( false, socket1.IsReadyForWriting() );

	assertEqual( false, socket1.IsReadyForReading() );

	assertEqual( false, socket1.ShutDown() );
	assertEqual( false, socket1.ShutDownReading() );
	assertEqual( false, socket1.ShutDownWriting() );
	assertEqual( false, socket1.SetNoDelay() );
}

void SocketTest::httpClientTest()
{
	Connector connector(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port"].AsLong());
	Socket *socket = connector.MakeSocket();

	assertEqual( GetConfig()["Testhost"]["ip"].AsString(), connector.GetAddress() );
	assertEqual( GetConfig()["Testhost"]["port"].AsLong(), connector.fPort );
	assertEqual( (long)NULL, (long)connector.fSocket );

	if ( t_assert( socket != NULL ) ) { // fails without HTTP Server
		long socketfd = socket->GetFd();
		t_assert( socketfd > 0L );

		// this one sets the connect timeout
		socket->SetTimeout(GetConfig()["GetStreamTimeout"].AsLong(5000L));
		iostream *Ios = socket->GetStream();
		t_assert( Ios != NULL);
		if ( Ios ) {
			String query("GET / HTTP/1.0");
			String reply;
			long lRetCode;
			if ( t_assertm(socket->IsReadyForWriting(GetConfig()["ReadyForWritingTimeout"].AsLong(5000L), lRetCode), TString("expected no timeout for sending http request to [") << connector.GetAddress() << ':' << connector.fPort << "]") && t_assert(lRetCode > 0) ) {
				(*Ios) << query << endl << endl;
				t_assert(!!(*Ios)); // make sure Ios is valid
				if ( t_assertm(socket->IsReadyForReading(GetConfig()["ReadyForReadingTimeout"].AsLong(5000L), lRetCode), TString("expected no timeout for reading HTTP reply [") << connector.GetAddress() << ':' << connector.fPort << "]") && t_assert(lRetCode > 0) ) {
					(*Ios) >> reply;
					assertEqual( "HTTP", reply.SubString(0, 4) ); // test first line of reply by http server
					t_assert(!!(*Ios));
				}
			}
		}
		delete socket;
		socket = 0;
		assertEqual( -1L, ::closeSocket(socketfd)); // did we really close the socket??
	}
}

void SocketTest::faultyClientTest()
{
	Connector connector(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["faultyport"].AsLong(), GetConfig()["ConnectorTimeout"].AsLong(10000L));  // this port should not be in use check with /etc/services
	Socket *socket = connector.MakeSocket();

	assertEqual( GetConfig()["Testhost"]["ip"].AsString(), connector.GetAddress() );
	assertEqual( GetConfig()["Testhost"]["faultyport"].AsLong(), connector.fPort );

	t_assert( socket == NULL );
	delete socket;
}

void SocketTest::MakeInetAddrTest()
{
	ROAnything roTestConfig = GetConfig()["MakeInetAddrTest"]["IpAddr"];
	for (long i = 0; i < roTestConfig.GetSize(); i++) {
		assertEqualm(roTestConfig[i].AsLong(), ntohl(EndPoint::MakeInetAddr(roTestConfig.SlotName(i))), TString("Failed at Addr [") << roTestConfig.SlotName(i) << "]");
	}
	unsigned long localInetAddr(GetConfig()["MakeInetAddrTest"]["LocalHost"].AsLong(0L));
	assertEqualm( localInetAddr , ntohl(EndPoint::MakeInetAddr("")), "Expected localhosts addr");
	assertEqualm( INADDR_ANY , EndPoint::MakeInetAddr("", true), "Expected any ip addr");
}

void SocketTest::AppendToClientInfoTest()
{
	Anything clnInfo;
	clnInfo["REMOTE_ADDR"] = "127.0.0.1";
	clnInfo["REMOTE_PORT"] = 1234L;
	Socket sock(-1, clnInfo);
	sock.AppendToClientInfo("ServerNonce", Anything("Rnd8Byts"));
	ROAnything ci(sock.ClientInfo());
	assertEqual("Rnd8Byts", ci["ServerNonce"].AsCharPtr("oops"));
}

bool SocketTest::IsNonBlocking(int fd)
{
#if defined(WIN32)
	return true;
#else
	int flags;
	if (fd >= 0 && (flags = fcntl(fd, F_GETFL, 0)) >= 0) {
		return 0 != (flags & O_NONBLOCK);
	}
	return false;
#endif
}

void SocketTest::SetToNonBlockingTest()
{
#if defined(WIN32)
	t_assertm(false, "set to non blocking only possible for sockets - ignore");
#else
	// invalid fd
	t_assert(!Socket::SetToNonBlocking(-1));
	t_assert(!IsNonBlocking(-1));
	t_assert(!IsNonBlocking(0));
	t_assert(Socket::SetToNonBlocking(0));
	t_assert(IsNonBlocking(0));
	t_assert(Socket::SetToNonBlocking(0, false));
	t_assert(!IsNonBlocking(0));
#endif
}

Test *SocketTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SocketTest, simpleConstructorTest);
	ADD_CASE(testSuite, SocketTest, allocatorConstructorTest);
	ADD_CASE(testSuite, SocketTest, faultyConstructorTest);
	ADD_CASE(testSuite, SocketTest, httpClientTest);
	ADD_CASE(testSuite, SocketTest, faultyClientTest);
	ADD_CASE(testSuite, SocketTest, MakeInetAddrTest);
	ADD_CASE(testSuite, SocketTest, AppendToClientInfoTest);
	ADD_CASE(testSuite, SocketTest, SetToNonBlockingTest);

	return testSuite;
}

