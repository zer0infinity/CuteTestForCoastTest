/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ListenerPoolTest.h"

//--- module under test --------------------------------------------------------
#include "RequestListener.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Dbg.h"

class TestReceiver
{
public:
	TestReceiver(): fMutex("MMMM") {}
	void Receive(iostream *Ios);
	Anything GetResult();

protected:
	Anything fResult;
	Mutex fMutex;
};

class TestCallBackFactory : public CallBackFactory
{
public:
	TestCallBackFactory() { };
	virtual ~TestCallBackFactory() {};
	virtual AcceptorCallBack *MakeCallBack();
	Anything GetResult() {
		return fReceiver.GetResult();
	}
protected:
	TestReceiver fReceiver;
};

class TestCallBack : public AcceptorCallBack
{
public:
	TestCallBack(TestReceiver *tr) : fReceiver(tr) {};
	virtual ~TestCallBack() {};
	virtual void CallBack(Socket *socket);

protected:
	TestReceiver *fReceiver;

};

AcceptorCallBack *TestCallBackFactory::MakeCallBack()
{
	return new TestCallBack(&fReceiver);
}

void TestCallBack::CallBack(Socket *socket)
{
	StartTrace(TestCallBack.CallBack);

	iostream *Ios = socket->GetStream();

	if (Ios) {
		fReceiver->Receive(Ios);
	}

	delete socket;
}

void TestReceiver::Receive(iostream *Ios)
{
	StartTrace(TestReceiver.Receive);

	MutexEntry me(fMutex);
	me.Use();
	String msg;
	*Ios >> msg;
	Ios->flush();
	fResult.Append(msg);
	*Ios << "Ok " << flush;
}

Anything TestReceiver::GetResult()
{
	MutexEntry me(fMutex);
	me.Use();
	return fResult;
}

class FakeAcceptorFactory: public AcceptorFactory
{
public:
	FakeAcceptorFactory(const char *name): AcceptorFactory(name) {};
	~FakeAcceptorFactory() {};

	//-- Cloning interface
	IFAObject *Clone() const {
		return new FakeAcceptorFactory(fName);
	}

	Acceptor *MakeAcceptor(AcceptorCallBack *ac) {
		return 0;
	}

protected:

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	FakeAcceptorFactory();
	FakeAcceptorFactory(const FakeAcceptorFactory &);
	FakeAcceptorFactory &operator=(const FakeAcceptorFactory &);

};
RegisterAcceptorFactory(FakeAcceptorFactory);

//---- ListenerPoolTest ----------------------------------------------------------------
ListenerPoolTest::ListenerPoolTest(TString tname)
	: ConfiguredTestCase(tname, "Config")
{
	StartTrace(ListenerPoolTest.Ctor);
}

ListenerPoolTest::~ListenerPoolTest()
{
	StartTrace(ListenerPoolTest.Dtor);
}

void ListenerPoolTest::setUp ()
{
	StartTrace(ListenerPoolTest.setUp);
	ConfiguredTestCase::setUp();
	t_assert(fConfig.IsDefined("AcceptorFactories"));
	t_assert(fConfig.IsDefined("Modules"));

	// ensure installation of modules
	t_assert(WDModule::Install(fConfig) == 0);
} // setUp

void ListenerPoolTest::tearDown ()
{
	StartTrace(ListenerPoolTest.tearDown);
	WDModule::Terminate(fConfig);
	ConfiguredTestCase::tearDown();
}

void ListenerPoolTest::PoolTest()
{
	StartTrace(ListenerPoolTest.PoolTest);

	Anything config;
	config.Append("TCP5010");
	config.Append("TCP5011");
	config.Append("TCP5012");
	config.Append("TCP5013");
	config.Append("TCP5014");

	TestCallBackFactory *tcbf = new TestCallBackFactory;
	ListenerPool lpToTest(tcbf);

	if ( t_assertm( lpToTest.Init(config.GetSize(), config), "Init should work") && t_assertm(lpToTest.Start(false, 0, 0) == 0, "Start should work")) {
		DoTestConnect();
		if (t_assertm(lpToTest.Terminate(1, 10) == 0, "Terminate failed")) {
			t_assertm(lpToTest.Join() == 0, "Join failed");
			Anything result = tcbf->GetResult();
			t_assert(result.Contains("5010_timeout_0"));
			t_assert(result.Contains("5011_timeout_0"));
			t_assert(result.Contains("5012_timeout_0"));
			t_assert(result.Contains("5010_timeout_1000"));
			t_assert(result.Contains("5011_timeout_1000"));
			t_assert(result.Contains("5012_timeout_1000"));
		}
	}
}

void ListenerPoolTest::DoSendReceive(Connector *c, String msg)
{
	StartTrace(ListenerPoolTest.DoSendReceive);

	iostream *Ios = c->GetStream();
	String errorMsg = msg;
	errorMsg  << msg << " connect failed";
	t_assertm(Ios != 0, (const char *)errorMsg);
	if (Ios) {
		*Ios << msg << endl;
		t_assert(!!(*Ios));
		String reply;
		*Ios >> reply;
		t_assertm(!!(*Ios), "oops socket time out? connector time out too short?");
		assertEqual("Ok", reply); //PS use correct macro
	}
}

void ListenerPoolTest::DoSendReceiveWithFailure(Connector *c, String msg, bool iosGoodAfterSend)
{
	StartTrace(ListenerPoolTest.DoSendReceive);
	iostream *Ios = c->GetStream();
	t_assertm(Ios != 0, "Expected iostream not to be 0");
	if (Ios) {
		*Ios << msg << endl;
		if (iosGoodAfterSend) {
			t_assertm(!!(*Ios), "Expected iostream state to be good after send");
		} else {
			t_assertm(!(*Ios), "Expected iostream state to be bad  after send");
		}
		String reply;
		*Ios >> reply;
		t_assertm(Ios->fail(), "Expected iostream state to be fail.");
	}
}

void ListenerPoolTest::InitFailureTest()
{
	StartTrace(ListenerPoolTest.InitFailureTest);

	Anything config;
	config.Append("TCP5010");
	config.Append("TCP5010");	// same port twice - should fail on init

	TestCallBackFactory *tcbf = new TestCallBackFactory;
	ListenerPool lpToTest(tcbf);

	t_assertm(!lpToTest.Init(config.GetSize(), config), "Init successful where it should fail");
}

void ListenerPoolTest::InitFailureNullAcceptorTest()
{
	StartTrace(ListenerPoolTest.InitFailureNullAcceptorTest);

	Anything config;
	config.Append("Fake");

	TestCallBackFactory *tcbf = new TestCallBackFactory;
	ListenerPool lpToTest(tcbf);

	t_assertm(!lpToTest.Init(config.GetSize(), config), "Init successful where it should fail");
}

void ListenerPoolTest::NullCallBackFactoryTest()
{
	StartTrace(ListenerPoolTest.NullCallBackFactoryTest);

	Anything config;
	config.Append("TCP5010");
	config.Append("TCP5010");	// same port twice - should fail on init

	TestCallBackFactory *tcbf = 0;
	ListenerPool lpToTest(tcbf);

	t_assertm(!lpToTest.Init(config.GetSize(), config), "Init successful where it should fail");
}
