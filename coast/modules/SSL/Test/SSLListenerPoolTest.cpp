/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "SSLSocket.h"
#include "SSLModule.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "RequestListener.h"

//--- interface include --------------------------------------------------------
#include "SSLListenerPoolTest.h"
#include "ListenerPoolTest.h"

//---- SSLListenerPoolTest ----------------------------------------------------------------
SSLListenerPoolTest::SSLListenerPoolTest(TString tname) : ListenerPoolTest(tname)
{
	StartTrace(SSLListenerPoolTest.Ctor);
}

SSLListenerPoolTest::~SSLListenerPoolTest()
{
	StartTrace(SSLListenerPoolTest.Dtor);
}

void SSLListenerPoolTest::PoolTest()
{
	StartTrace(SSLListenerPoolTest.PoolTest);
	Anything config;
	config.Append("TCP5010");
	config.Append("TCP5011");
	config.Append("TCP5012");
	config.Append("TCP5013");
	config.Append("TCP5014");
	config.Append("TCP5015");
	config.Append("TCP5016");
	config.Append("TCP5017");
	config.Append("TCP5018");
	config.Append("TCP5019");

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
			t_assert(result.Contains("5013_timeout_0"));
			t_assert(result.Contains("5014_timeout_0"));
			t_assert(result.Contains("5010_timeout_1000"));
			t_assert(result.Contains("5011_timeout_1000"));
			t_assert(result.Contains("5012_timeout_1000"));
			t_assert(result.Contains("5012_timeout_2000"));
			t_assert(result.Contains("5011_timeout_2000"));
			t_assert(result.Contains("5016_timeout_1000"));
			t_assert(result.Contains("5017_timeout_1000"));
		}
	}
	Anything failures = tcbf->GetFailures();
	t_assertm(failures.GetSize() == 0, "Receivers encountered a least one error");
	if (failures.GetSize()) {
		TraceAny(failures, "EYECATCHER Receivers encountered the following errors");
	}
}

void SSLListenerPoolTest::DoTestConnect()
{
	StartTrace(SSLListenerPoolTest.DoTestConnect);
	FOREACH_ENTRY("SSLListenerPoolTest", cConfig, cName) {
		TraceAny(cConfig, "cConfig");
		Trace(cConfig["SSLConnector"].AsLong(1));
		if ( cConfig["SSLConnector"].AsLong(1) == 1 ) {
			if ( cConfig["ConnectorToUse"].AsString() != "" ) {
				// deep clone, no side effect when adding Timeout
				Anything connectorConfig = fTestCaseConfig[cConfig["ConnectorToUse"].AsString()].DeepClone();
				if ( cConfig.IsDefined("TimeoutToUse") ) {
					connectorConfig["Timeout"] = 1000L;
				}
				SSLConnector sc(connectorConfig);
				if ( cConfig["DoSendReceiveWithFailure"].AsLong(0) ) {
					DoSendReceiveWithFailure(&sc, cConfig["Data"].DeepClone(),
											 cConfig["IOSGoodAfterSend"].AsLong(0),
											 cConfig["IOSGoodBeforeSend"].AsLong(1));
				} else {
					DoSendReceive(&sc, cConfig["Data"].DeepClone());
				}
			} else {
				SSLConnector sc("localhost", cConfig["PortToUse"].AsLong(0), cConfig["TimeoutToUse"].AsLong(0));
				if ( cConfig["DoSendReceiveWithFailure"].AsLong(0) ) {
					DoSendReceiveWithFailure(&sc, cConfig["Data"].DeepClone(),
											 cConfig["IOSGoodAfterSend"].AsLong(0),
											 cConfig["IOSGoodBeforeSend"].AsLong(1));
				} else {
					DoSendReceive(&sc, cConfig["Data"].DeepClone());
				}
			}
		} else {
			Trace("Using configured NON SSL connector");
			Connector c("localhost", cConfig["PortToUse"].AsLong(0L), cConfig["TimeoutToUse"].AsLong(0L));
			if ( cConfig["DoSendReceiveWithFailure"].AsLong(0) ) {
				DoSendReceiveWithFailure(&c, cConfig["Data"].DeepClone(),
										 cConfig["IOSGoodAfterSend"].AsLong(0),
										 cConfig["IOSGoodBeforeSend"].AsLong(1));
			} else {
				DoSendReceive(&c, cConfig["Data"].DeepClone());
			}
		}
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *SSLListenerPoolTest::suite ()
{
	StartTrace(SSLListenerPoolTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(SSLListenerPoolTest, PoolTest));
//#if !defined(WIN32)
//	testSuite->addTest (NEW_CASE(SSLListenerPoolTest, InitFailureTest)); // test does not work on WIN32
//#endif
	testSuite->addTest (NEW_CASE(SSLListenerPoolTest, NullCallBackFactoryTest));
	testSuite->addTest (NEW_CASE(SSLListenerPoolTest, InitFailureNullAcceptorTest));
	return testSuite;

} // suite
