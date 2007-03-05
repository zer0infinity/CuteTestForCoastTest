/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LFListenerPoolTest.h"

//--- module under test --------------------------------------------------------
#include "LFListenerPool.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Socket.h"
#include "WPMStatHandler.h"
#include "RequestProcessor.h"

//--- TestRequestReactor ----
//:test driver for lf pool testing
class TestProcessor: public RequestProcessor
{
public:
	TestProcessor(LFListenerPoolTest *);
	~TestProcessor();
	void ProcessRequest(Context &ctx);

protected:
	LFListenerPoolTest *fTest;
};

TestProcessor::TestProcessor(LFListenerPoolTest *test)
	: RequestProcessor("TestProcessor")
	, fTest(test)
{
	StartTrace(TestProcessor.Ctor);
}

TestProcessor::~TestProcessor()
{
	StartTrace(TestProcessor.Dtor);
}

void TestProcessor::ProcessRequest(Context &ctx)
{
	StartTrace(TestProcessor.ProcessRequest);
	fTest->EventProcessed(ctx.GetSocket());
}

//---- LFListenerPoolTest ----------------------------------------------------------------
LFListenerPoolTest::LFListenerPoolTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(LFListenerPoolTest.LFListenerPoolTest);
}

LFListenerPoolTest::~LFListenerPoolTest()
{
	StartTrace(LFListenerPoolTest.Dtor);
}

void LFListenerPoolTest::setUp ()
{
	StartTrace(LFListenerPoolTest.setUp);
	t_assert(GetConfig().IsDefined("Modules"));
}

void LFListenerPoolTest::NoReactorTest()
{
	StartTrace(LFListenerPoolTest.NoReactorTest);
	LFListenerPool lfp(0);

	Trace("ip:" << GetConfig()["Testhost"]["ip"].AsString() << " : port:" << GetConfig()["Testhost"]["port"].AsLong());
	Acceptor ac1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port"].AsLong(), 5, 0);
	Anything lfpConfig;
	lfpConfig[String("Accept") << GetConfig()["Testhost"]["port"].AsString()] = (IFAObject *)&ac1;
	if ( t_assertm( !lfp.Init(2, lfpConfig, true), "no reactor is configured; init should fail")) {
		t_assertm(lfp.GetPoolSize() == 0, "expected no threads in pool");
		t_assertm(lfp.Start(true, 1000, 11) == -1, "expected Start to fail");
	}
}

void LFListenerPoolTest::NoFactoryTest()
{
	StartTrace(LFListenerPoolTest.NoFactoryTest);
	const long cNumOfThreads = 2;
	LFListenerPool lfp(new RequestReactor(new TestProcessor(this), new WPMStatHandler(cNumOfThreads)));

	Anything lfpConfig;
	lfpConfig.Append("TCP4343");
	if (t_assertm(!lfp.Init(cNumOfThreads, lfpConfig, true), "expected initialization to fail; no valid factory")) {
		t_assertm(lfp.GetPoolSize() == 0, "expected no threads in pool");
		t_assertm(lfp.Start(true, 1000, 11) == -1, "expected Start to fail");
	}
}

void LFListenerPoolTest::InvalidAcceptorTest()
{
	StartTrace(LFListenerPoolTest.InvalidAcceptorTest);
	const long cNumOfThreads = 2;
	LFListenerPool lfp(new RequestReactor(new TestProcessor(this), new WPMStatHandler(cNumOfThreads)));

	Anything lfpConfig;
	lfpConfig.Append("Fake");
	if (t_assertm(!lfp.Init(cNumOfThreads, lfpConfig, true), "expected initialization to fail; acceptor is not valid")) {
		t_assertm(lfp.GetPoolSize() == 0, "expected no threads in pool");
		t_assertm(lfp.Start(true, 1000, 11) == -1, "expected Start to fail");
	}
}

void LFListenerPoolTest::InvalidReactorTest()
{
	StartTrace(LFListenerPoolTest.InvalidReactorTest);
	const long cNumOfThreads = 2;
	LFListenerPool lfp(new RequestReactor(0, 0));

	Anything lfpConfig;
	lfpConfig.Append("TCP5010");
	if (t_assertm(!lfp.Init(cNumOfThreads, lfpConfig, true), "expected initialization to fail; reactor is not valid")) {
		t_assertm(lfp.GetPoolSize() == 0, "expected no threads in pool");
		t_assertm(lfp.Start(true, 1000, 11) == -1, "expected Start to fail");
	}
}

void LFListenerPoolTest::OneAcceptorTest()
{
	StartTrace(LFListenerPoolTest.OneAcceptorTest);
	const long cNumOfThreads = 2;
	LFListenerPool lfp(new RequestReactor(new TestProcessor(this), new WPMStatHandler(cNumOfThreads)));

	Anything lfpConfig;
	lfpConfig.Append("TCP5010");
	if (t_assertm(lfp.Init(cNumOfThreads, lfpConfig, true), "expected initialization to succeed; are factories present?")) {
		if (t_assert(lfp.Start(true, 1000, 11) == 0 )) {
			ProcessOneEvent();
			lfp.RequestTermination();
			t_assertm(lfp.Join() == 0, "expected Join to succeed");
		}
	}
}

void LFListenerPoolTest::TwoAcceptorsTest()
{
	StartTrace(LFListenerPoolTest.TwoAcceptorsTest);
	const long cNumOfThreads = 3;
	LFListenerPool lfp(new RequestReactor(new TestProcessor(this), new WPMStatHandler(cNumOfThreads)));

	Anything lfpConfig;
	lfpConfig.Append("TCP5010");
	lfpConfig.Append("TCP5011");
	if (t_assertm(lfp.Init(cNumOfThreads, lfpConfig, true), "expected initialization to succeed; are factories present?")) {
		if (t_assert(lfp.Start(true, 1000, 11) == 0 )) {
			ProcessTwoEvents();
			lfp.RequestTermination();
			t_assertm(lfp.Join() == 0, "expected Join to succeed");
		}
	}
}

void LFListenerPoolTest::ManyAcceptorsTest()
{
	StartTrace(LeaderFollowerPoolTest.ManyAcceptorsTest);
	{
		const long cNumOfThreads = 0;
		LFListenerPool lfp(new RequestReactor(new TestProcessor(this), new WPMStatHandler(cNumOfThreads)));

		Anything lfpConfig;
		lfpConfig.Append("TCP5010");
		lfpConfig.Append("TCP5011");
		lfpConfig.Append("TCP5012");

		if (t_assertm(!lfp.Init(cNumOfThreads, lfpConfig, true), "should not allow LFPool to run without threads")) {
			t_assertm(lfp.GetPoolSize() == 0, "expected no threads in pool");
			t_assertm(lfp.Start(true, 1000, 11) == -1, "expected Start to fail");

		}
	}
	{
		const long cNumOfThreads = 1;
		LFListenerPool lfp(new RequestReactor(new TestProcessor(this), new WPMStatHandler(cNumOfThreads)));

		Anything lfpConfig;
		lfpConfig.Append("TCP5010");
		lfpConfig.Append("TCP5011");
		lfpConfig.Append("TCP5012");

		if (t_assertm(lfp.Init(cNumOfThreads, lfpConfig, true), "some port maybe already bound")) {
			if (t_assert(lfp.Start(false, 1000, 10) == 0 )) {
				ProcessManyEvents();
				lfp.RequestTermination();
				t_assertm(lfp.Join() == 0, "expected Join to succeed");
			}
		}
	}
	{
		const long cNumOfThreads = 2;
		LFListenerPool lfp(new RequestReactor(new TestProcessor(this), new WPMStatHandler(cNumOfThreads)));

		Anything lfpConfig;
		lfpConfig.Append("TCP5010");
		lfpConfig.Append("TCP5011");
		lfpConfig.Append("TCP5012");

		if (t_assertm(lfp.Init(cNumOfThreads, lfpConfig, true), "some port maybe already bound")) {
			if (t_assert(lfp.Start(false, 1000, 10) == 0 )) {
				ProcessManyEvents();
				lfp.RequestTermination();
				t_assertm(lfp.Join() == 0, "expected Join to succeed");
			}
		}
	}
	{
		const long cNumOfThreads = 3;
		LFListenerPool lfp(new RequestReactor(new TestProcessor(this), new WPMStatHandler(cNumOfThreads)));

		Anything lfpConfig;
		lfpConfig.Append("TCP5010");
		lfpConfig.Append("TCP5011");
		lfpConfig.Append("TCP5012");

		if (t_assertm(lfp.Init(cNumOfThreads, lfpConfig, true), "some port maybe already bound")) {
			if (t_assert(lfp.Start(false, 1000, 10) == 0 )) {
				ProcessManyEvents();
				lfp.RequestTermination();
				t_assertm(lfp.Join() == 0, "expected Join to succeed");
			}
		}
	}
	{
		const long cNumOfThreads = 4;
		LFListenerPool lfp(new RequestReactor(new TestProcessor(this), new WPMStatHandler(cNumOfThreads)));

		Anything lfpConfig;
		lfpConfig.Append("TCP5010");
		lfpConfig.Append("TCP5011");
		lfpConfig.Append("TCP5012");

		if (t_assertm(lfp.Init(cNumOfThreads, lfpConfig, true), "some port maybe already bound")) {
			if (t_assert(lfp.Start(true, 1000, 11) == 0 )) {
				ProcessManyEvents();
				lfp.RequestTermination();
				t_assertm(lfp.Join() == 0, "expected Join to succeed");
			}
		}
	}
}

bool LFListenerPoolTest::EventProcessed(Socket *socket)
{
	StartTrace(LFListenerPoolTest.EventProcessed);
	if (t_assert(socket != NULL)) {
		String request;
		(*socket->GetStream()) >> request;
		assertEqual("hallo", request);
		(*socket->GetStream()) << "HostReply" << endl;
		return true;
	}
	return false;
}

void LFListenerPoolTest::ProcessOneEvent()
{
	StartTrace(LeaderFollowerPoolTest.ProcessOneEvent);
	Connector c1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["TCP5010"]["Port"].AsLong());
	String reply1;

	if ( t_assert(c1.GetStream() != NULL) ) {
		(*c1.GetStream()) << "hallo" << endl;
		t_assert(!!(*c1.GetStream()));
		(*c1.GetStream()) >> reply1;
		t_assert(!!(*c1.GetStream()));

		assertEqual("HostReply", reply1);
	}
}

void LFListenerPoolTest::ProcessManyEvents()
{
	StartTrace(LeaderFollowerPoolTest.ProcessManyEvents);
	Connector c1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["TCP5010"]["Port"].AsLong());
	Connector c2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["TCP5011"]["Port"].AsLong());
	Connector c3(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["TCP5012"]["Port"].AsLong());
	String reply1;
	String reply2;
	String reply3;

	if (t_assert(c1.GetStream() != NULL) && t_assert(c2.GetStream() != NULL) && t_assert(c3.GetStream() != NULL)) {
		(*c1.GetStream()) << "hallo" << endl;
		(*c2.GetStream()) << "hallo" << endl;
		(*c3.GetStream()) << "hallo" << endl;
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

void LFListenerPoolTest::ProcessTwoEvents()
{
	StartTrace(LeaderFollowerPoolTest.ProcessTwoEvents);
	Connector c1(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["TCP5010"]["Port"].AsLong());
	Connector c2(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["TCP5011"]["Port"].AsLong());
	String reply1;
	String reply2;

	if (t_assert(c1.GetStream() != NULL) && t_assert(c2.GetStream() != NULL)) {
		(*c1.GetStream()) << "hallo" << endl;
		(*c2.GetStream()) << "hallo" << endl;
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

// builds up a suite of testcases, add a line for each testmethod
Test *LFListenerPoolTest::suite ()
{
	StartTrace(LFListenerPoolTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, LFListenerPoolTest, NoReactorTest);
	ADD_CASE(testSuite, LFListenerPoolTest, NoFactoryTest);
	ADD_CASE(testSuite, LFListenerPoolTest, OneAcceptorTest);
	ADD_CASE(testSuite, LFListenerPoolTest, TwoAcceptorsTest);
	ADD_CASE(testSuite, LFListenerPoolTest, ManyAcceptorsTest);
	ADD_CASE(testSuite, LFListenerPoolTest, InvalidAcceptorTest);
	ADD_CASE(testSuite, LFListenerPoolTest, InvalidReactorTest);

	return testSuite;
}
