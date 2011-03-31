/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LeaderFollowerPoolTest.h"
#include "LeaderFollowerPool.h"
#include "TestSuite.h"
#include "Socket.h"
#if !defined(WIN32)
#include <sys/poll.h>
#endif

//--- TestReactor ----
//:test driver for lf pool testing
class TestReactor: public Reactor
{
public:
	TestReactor(LeaderFollowerPoolTest *);
	~TestReactor();
	virtual void DoProcessEvent(Socket *s);

protected:
	LeaderFollowerPoolTest *fTest;
};

//---- LeaderFollowerPoolTest ----------------------------------------------------------------
LeaderFollowerPoolTest::LeaderFollowerPoolTest(TString tstrName)
	: TestCaseType(tstrName)
	, fEvents(0)
{
	StartTrace(LeaderFollowerPoolTest.Ctor);
}

LeaderFollowerPoolTest::~LeaderFollowerPoolTest()
{
	StartTrace(LeaderFollowerPoolTest.Dtor);
}

void LeaderFollowerPoolTest::NoReactorTest()
{
	StartTrace(LeaderFollowerPoolTest.NoReactorTest);
	LeaderFollowerPool lfp(0);

	Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong(), 5, 0);
	Anything lfpConfig;
	lfpConfig[String("Accept") << GetConfig()["Testhost"]["port1"].AsString()] = (IFAObject *)&ac1;
	if ( t_assertm( !lfp.Init(2, lfpConfig), "no reactor is configured; init should fail") ) {
		t_assertm(lfp.GetPoolSize() == 0, "expected no threads in pool");
		t_assertm(lfp.Start(true, 1000, 11) == -1, "expected Start to fail");
	}
}

void LeaderFollowerPoolTest::NoAcceptorsTest()
{
	StartTrace(LeaderFollowerPoolTest.NoAcceptorsTest);
	LeaderFollowerPool lfp(new TestReactor(this));

	Anything lfpConfig;
	if ( t_assertm( !lfp.Init(2, lfpConfig), "no acceptors are configured; init should fail") ) {
		t_assertm(lfp.GetPoolSize() == 0, "expected no threads in pool");
		t_assertm(lfp.Start(true, 1000, 11) == -1, "expected Start to fail");
	}
}

void LeaderFollowerPoolTest::OneAcceptorTest()
{
	StartTrace(LeaderFollowerPoolTest.OneAcceptorTest);

	LeaderFollowerPool lfp(new TestReactor(this));

	Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong(), 5, 0);
	Anything lfpConfig;
	lfpConfig[String("Accept") << GetConfig()["Testhost"]["port1"].AsString()] = (IFAObject *)&ac1;
	if ( t_assertm( lfp.Init(3, lfpConfig), "some port maybe already bound")) {
		if (t_assert(lfp.Start(true, 1000, 11) == 0 )) {
			ProcessOneEvent();
			lfp.RequestTermination();
			t_assertm(lfp.Join() == 0, "expected Join to succeed");
		}
	}
}

void LeaderFollowerPoolTest::TwoAcceptorsTest()
{
	StartTrace(LeaderFollowerPoolTest.TwoAcceptorsTest);

	LeaderFollowerPool lfp(new TestReactor(this));

	Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong(), 5, 0);
	Acceptor ac2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port2"].AsLong(), 5, 0);
	Anything lfpConfig;
	lfpConfig[String("Accept") << GetConfig()["Testhost"]["port1"].AsString()] = (IFAObject *)&ac1;
	lfpConfig[String("Accept") << GetConfig()["Testhost"]["port2"].AsString()] = (IFAObject *)&ac2;
	if ( t_assertm( lfp.Init(2, lfpConfig), "some port maybe already bound")) {
		if (t_assert(lfp.Start(false, 1000, 10) == 0 )) {
			ProcessTwoEvents();
			lfp.RequestTermination();
			t_assertm(lfp.Join() == 0, "expected Join to succeed");
		}
	}
}

void LeaderFollowerPoolTest::ManyAcceptorsTest()
{
	StartTrace(LeaderFollowerPoolTest.ManyAcceptorsTest);
	{
		LeaderFollowerPool lfp(new TestReactor(this));

		Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong(), 5, 0);
		Acceptor ac2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port2"].AsLong(), 5, 0);
		Acceptor ac3(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port3"].AsLong(), 5, 0);
		Anything lfpConfig;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port1"].AsString()] = (IFAObject *)&ac1;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port2"].AsString()] = (IFAObject *)&ac2;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port3"].AsString()] = (IFAObject *)&ac3;
		if ( t_assertm( !lfp.Init(0, lfpConfig), "should not allow LFPool to run without threads") ) {
			t_assertm(lfp.GetPoolSize() == 0, "expected no threads in pool");
			t_assertm(lfp.Start(true, 1000, 11) == -1, "expected Start to fail");
		}
	}
	{
		LeaderFollowerPool lfp(new TestReactor(this));

		Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong(), 5, 0);
		Acceptor ac2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port2"].AsLong(), 5, 0);
		Acceptor ac3(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port3"].AsLong(), 5, 0);
		Anything lfpConfig;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port1"].AsString()] = (IFAObject *)&ac1;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port2"].AsString()] = (IFAObject *)&ac2;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port3"].AsString()] = (IFAObject *)&ac3;
		if ( t_assertm( lfp.Init(1, lfpConfig), "some port maybe already bound")) {
			if (t_assert(lfp.Start(false, 1000, 10) == 0 )) {
				ProcessManyEvents();
				lfp.RequestTermination();
				t_assertm(lfp.Join() == 0, "expected Join to succeed");
			}
		}
	}
	{
		LeaderFollowerPool lfp(new TestReactor(this));

		Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong(), 5, 0);
		Acceptor ac2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port2"].AsLong(), 5, 0);
		Acceptor ac3(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port3"].AsLong(), 5, 0);
		Anything lfpConfig;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port1"].AsString()] = (IFAObject *)&ac1;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port2"].AsString()] = (IFAObject *)&ac2;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port3"].AsString()] = (IFAObject *)&ac3;
		if ( t_assertm( lfp.Init(2, lfpConfig), "some port maybe already bound")) {
			if (t_assert(lfp.Start(false, 1000, 10) == 0 )) {
				ProcessManyEvents();
				lfp.RequestTermination();
				t_assertm(lfp.Join() == 0, "expected Join to succeed");
			}
		}
	}
	{
		LeaderFollowerPool lfp(new TestReactor(this));

		Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong(), 5, 0);
		Acceptor ac2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port2"].AsLong(), 5, 0);
		Acceptor ac3(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port3"].AsLong(), 5, 0);
		Anything lfpConfig;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port1"].AsString()] = (IFAObject *)&ac1;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port2"].AsString()] = (IFAObject *)&ac2;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port3"].AsString()] = (IFAObject *)&ac3;
		if ( t_assertm( lfp.Init(3, lfpConfig), "some port maybe already bound")) {
			if (t_assert(lfp.Start(true, 1000, 11) == 0 )) {
				ProcessManyEvents();
				lfp.RequestTermination();
				t_assertm(lfp.Join() == 0, "expected Join to succeed");
			}
		}
	}
	{
		LeaderFollowerPool lfp(new TestReactor(this));

		Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong(), 5, 0);
		Acceptor ac2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port2"].AsLong(), 5, 0);
		Acceptor ac3(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port3"].AsLong(), 5, 0);
		Anything lfpConfig;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port1"].AsString()] = (IFAObject *)&ac1;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port2"].AsString()] = (IFAObject *)&ac2;
		lfpConfig[String("Accept") << GetConfig()["Testhost"]["port3"].AsString()] = (IFAObject *)&ac3;
		if ( t_assertm( lfp.Init(4, lfpConfig), "some port maybe already bound")) {
			if (t_assert(lfp.Start(true, 1000, 11) == 0 )) {
				ProcessManyEvents();
				lfp.RequestTermination();
				t_assertm(lfp.Join() == 0, "expected Join to succeed");
			}
		}
	}
}

void LeaderFollowerPoolTest::InvalidAcceptorTest()
{
	StartTrace(LeaderFollowerPoolTest.InvalidAcceptorTest);

	LeaderFollowerPool lfp(new TestReactor(this));

	Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port"].AsLong(), 5, 0);
	Anything lfpConfig;
	lfpConfig[String("Accept") << GetConfig()["Testhost"]["port"].AsString()] = (IFAObject *)&ac1;
	if ( t_assertm( !lfp.Init(3, lfpConfig), "should not bind to already bound port and needs root privilege anyway") ) {
		t_assertm(lfp.GetPoolSize() == 0, "expected no threads in pool");
		t_assertm(lfp.Start(true, 1000, 11) == -1, "expected Start to fail");
	}
}

void LeaderFollowerPoolTest::InvalidAcceptorsTest()
{
	StartTrace(LeaderFollowerPoolTest.InvalidAcceptorsTest);

	LeaderFollowerPool lfp(new TestReactor(this));

	Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong(), 5, 0);
	Acceptor ac2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port"].AsLong(), 5, 0);
	Anything lfpConfig;
	lfpConfig[String("Accept") << GetConfig()["Testhost"]["port1"].AsString()] = (IFAObject *)&ac1;
	lfpConfig[String("Accept") << GetConfig()["Testhost"]["port"].AsString()] = (IFAObject *)&ac2;
	if ( t_assertm( !lfp.Init(2, lfpConfig), "should not bind to already bound port and needs root privilege anyway") ) {
		t_assertm(lfp.GetPoolSize() == 0, "expected no threads in pool");
		t_assertm(lfp.Start(true, 1000, 11) == -1, "expected Start to fail");
	}
}

void LeaderFollowerPoolTest::ProcessTwoEvents()
{
	Connector c1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong());
	Connector c2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port2"].AsLong());
	String reply1;
	String reply2;

	if (t_assert(c1.GetStream() != NULL) && t_assert(c2.GetStream() != NULL)) {
		(*c1.GetStream()) << "hallo" << std::endl;
		(*c2.GetStream()) << "hallo" << std::endl;
		t_assert(!!(*c1.GetStream()));
		t_assert(!!(*c2.GetStream()));
		(*c1.GetStream()) >> reply1;
		(*c2.GetStream()) >> reply2;
		t_assert(!!(*c1.GetStream()));
		t_assert(!!(*c2.GetStream()));

		assertEqual("HostReply", reply1);
		assertEqual("HostReply", reply2);
	}
}

void LeaderFollowerPoolTest::ProcessManyEvents()
{
	Connector c1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong());
	Connector c2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port2"].AsLong());
	Connector c3(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port3"].AsLong());
	String reply1;
	String reply2;
	String reply3;

	if (t_assert(c1.GetStream() != NULL) &&
		t_assert(c2.GetStream() != NULL) &&
		t_assert(c3.GetStream() != NULL)) {
		(*c1.GetStream()) << "hallo" << std::endl;
		(*c2.GetStream()) << "hallo" << std::endl;
		(*c3.GetStream()) << "hallo" << std::endl;
		t_assert(!!(*c1.GetStream()));
		t_assert(!!(*c2.GetStream()));
		t_assert(!!(*c3.GetStream()));
		(*c1.GetStream()) >> reply1;
		(*c2.GetStream()) >> reply2;
		(*c3.GetStream()) >> reply3;
		t_assert(!!(*c1.GetStream()));
		t_assert(!!(*c2.GetStream()));
		t_assert(!!(*c3.GetStream()));

		assertEqual("HostReply", reply1);
		assertEqual("HostReply", reply2);
		assertEqual("HostReply", reply3);
	}
}

void LeaderFollowerPoolTest::ProcessOneEvent()
{
	StartTrace(LeaderFollowerPoolTest.ProcessOneEvent);
	Connector c1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port1"].AsLong());
	String reply1;

	if ( t_assert(c1.GetStream() != NULL) ) {
		(*c1.GetStream()) << "hallo" << std::endl;
		t_assert(!!(*c1.GetStream()));
		(*c1.GetStream()) >> reply1;
		t_assert(!!(*c1.GetStream()));

		assertEqual("HostReply", reply1);
	}
}

bool LeaderFollowerPoolTest::EventProcessed(Socket *socket)
{
	StartTrace(LeaderFollowerPoolTest.EventProcessed);
	if (t_assert(socket != NULL)) {
		String request;
		(*socket->GetStream()) >> request;
		assertEqual("hallo", request);
		(*socket->GetStream()) << "HostReply" << std::endl;
		return true;
	}
	return false;
}

Test *LeaderFollowerPoolTest::suite ()
{
	StartTrace(LeaderFollowerPoolTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, LeaderFollowerPoolTest, NoReactorTest);
	ADD_CASE(testSuite, LeaderFollowerPoolTest, NoAcceptorsTest);
	ADD_CASE(testSuite, LeaderFollowerPoolTest, OneAcceptorTest);
	ADD_CASE(testSuite, LeaderFollowerPoolTest, TwoAcceptorsTest);
	ADD_CASE(testSuite, LeaderFollowerPoolTest, ManyAcceptorsTest);
// WIN32 allows to bind the same port multiple times... so the following tests can not be done
#if !defined(WIN32)
	ADD_CASE(testSuite, LeaderFollowerPoolTest, InvalidAcceptorTest);
	ADD_CASE(testSuite, LeaderFollowerPoolTest, InvalidAcceptorsTest);
#endif
	return testSuite;
}

TestReactor::TestReactor(LeaderFollowerPoolTest *lfp)
	:	fTest(lfp)
{
	StartTrace(TestReactor.Ctor);
}

TestReactor::~TestReactor()
{
	StartTrace(TestReactor.Dtor);
}

void TestReactor::DoProcessEvent(Socket *socket)
{
	StartTrace(TestReactor.DoProcessEvent);
	fTest->EventProcessed(socket);
}
