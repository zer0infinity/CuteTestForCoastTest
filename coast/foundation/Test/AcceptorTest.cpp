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
#include "AcceptorTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"

//--- c-library modules used ---------------------------------------------------

//---- AcceptorTest ----------------------------------------------------------------
AcceptorTest::AcceptorTest(TString tname) : TestCase(tname)
{
}

AcceptorTest::~AcceptorTest()
{
}

void AcceptorTest::setUp ()
{
	istream *is = System::OpenStream("AcceptorTest", "any");
	if ( is ) {
		fConfig.Import( *is );
		delete is;
	} else {
		assertEqual( "'read AcceptorTest.any'", "'could not read AcceptorTest.any'" );
	}
} // setUp

void AcceptorTest::tearDown ()
{
} // tearDown
void AcceptorTest::dnsNameParam()
{
	Acceptor acceptor(fConfig["dnsNameParam"]["name"].AsString(), 0, 0, 0);
	assertEqual( fConfig["dnsNameParam"]["ip"].AsString(), acceptor.GetAddress() );
}

void AcceptorTest::getPort()
{
	Acceptor acceptor(fConfig["Localhost"]["ip"].AsString(), 0, 0, 0);

	assertEqual( 0, acceptor.GetPort() );
	assertEqual(0, acceptor.PrepareAcceptLoop());
	t_assert( 0 != acceptor.GetPort() );
	t_assert( 0 != acceptor.GetBoundPort());
	assertEqual(acceptor.GetPort(), acceptor.GetBoundPort());
}

class TestCallBack: public AcceptorCallBack
{
public:
	TestCallBack(AcceptorTest *test): fTest(test) { }
	~TestCallBack() { }
	void CallBack(Socket *socket) {
		if (fTest) {
			fTest->TestSocket(socket);
		}
		delete socket;
	}

protected:
	AcceptorTest *fTest;
};

void AcceptorTest::TestSocket(Socket *s)
{
	t_assert(s != 0);
}

void AcceptorTest::acceptOnceTest()
{
	// do not change the sequence of statements
	// the program cannot run otherwise
#ifdef __sun
	TestCallBack *cb = new TestCallBack(this);
	Acceptor acceptor(fConfig["acceptOnceTest"]["ip"].AsString(), 0, 1, cb);

	t_assert(0 == acceptor.PrepareAcceptLoop());
	t_assert( 0 != acceptor.GetPort() );

	Connector connector(fConfig["acceptOnceTest"]["ip"].AsString(), acceptor.GetPort());
	if (t_assert(connector.Use() != NULL)) { // should try a connect to acceptor; not yet accepting
		acceptor.RunAcceptLoop(true);
	}
#endif
	// if we get here we were succesful
}

Test *AcceptorTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AcceptorTest, getPort);
	ADD_CASE(testSuite, AcceptorTest, acceptOnceTest);

	return testSuite;
}
