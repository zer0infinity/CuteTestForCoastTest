/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ListenerPoolTest_H
#define _ListenerPoolTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"
#include "Socket.h"

class Connector;

//---- ListenerPoolTest ----------------------------------------------------------
//!TestCases description
class ListenerPoolTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	ListenerPoolTest(TString tstrName);

	//!destroys the test case
	~ListenerPoolTest();

	//--- public api

	//!must be implemented by subclasses since this class is abstract
	//! static Test *suite ();

	void setUp ();

	//!Everything works fine within this testcase
	void PoolTest();
	//!initialization fails
	void InitFailureTest();
	//!illegal constructor parameter
	void NullCallBackFactoryTest();
	//!acceptor can not be constructed
	void InitFailureNullAcceptorTest();

protected:
	// utility to test with non standard Connectors
	virtual void DoTestConnect() = 0;
	virtual void DoSendReceive(Connector *c, String msg);
	virtual void DoSendReceive(Connector *c, Anything toSend);
	virtual void DoSendReceiveWithFailure(Connector *c, String msg, bool iosGoodAfterSend, bool iosGoodBeforeSend);
	virtual void DoSendReceiveWithFailure(Connector *c, Anything toSend, bool iosGoodAfterSend, bool iosGoodBeforeSend);
};

class TestReceiver
{
public:
	TestReceiver(): fMutex("MMMM") {}
	void Receive(std::iostream *Ios, Socket *socket);
	Anything GetResult();
	Anything GetFailures();
	void DoChecks(Anything &toImport, Socket *socket);

protected:
	Anything fResult;
	Anything fFailures;
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
	Anything GetFailures() {
		return fReceiver.GetFailures();
	}
protected:
	TestReceiver fReceiver;
};

#endif
