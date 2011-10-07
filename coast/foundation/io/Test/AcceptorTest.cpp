/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AcceptorTest.h"
#include "TestSuite.h"
#include "Socket.h"

AcceptorTest::AcceptorTest(TString tname)
	: TestCaseType(tname)
{
}

AcceptorTest::~AcceptorTest()
{
}

void AcceptorTest::getPort()
{
	Acceptor acceptor(GetConfig()["Localhost"]["ip"].AsString(), 0, 0, 0);

	assertEqual( 0, acceptor.GetPort() );
	assertEqual(0, acceptor.PrepareAcceptLoop());
	assertCompare( 0L, not_equal_to, acceptor.GetPort() );
	assertCompare( 0L, not_equal_to, acceptor.GetBoundPort() );
	assertEqual(acceptor.GetPort(), acceptor.GetBoundPort());
}

class TestCallBack: public AcceptorCallBack
{
public:
	TestCallBack(AcceptorTest *test): fTest(test) { }

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
	Acceptor acceptor(GetConfig()["acceptOnceTest"]["ip"].AsString(), 0, 1, cb);

	assertCompare( 0, equal_to, acceptor.PrepareAcceptLoop() );
	assertCompare( 0L, not_equal_to, acceptor.GetPort() );

	Connector connector(GetConfig()["acceptOnceTest"]["ip"].AsString(), acceptor.GetPort());
	if (t_assert(connector.Use() != NULL)) { // should try a connect to acceptor; not yet accepting
		acceptor.RunAcceptLoop(true);
	}
#endif
	// if we get here we were succesful
}

Test *AcceptorTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AcceptorTest, getPort);
	ADD_CASE(testSuite, AcceptorTest, acceptOnceTest);

	return testSuite;
}
