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
	config.Append("TCP5020");

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
			t_assert(result.Contains("5020_timeout_0"));
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
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetConfig()["SSLListenerPoolTest"]);
	while ( aEntryIterator.Next(cConfig) ) {
		TraceAny(cConfig, "cConfig");
		Trace(cConfig["SSLConnector"].AsLong(1));
		Anything data = cConfig["Data"].DeepClone();
		long connectsToDo = cConfig["NumberOfConnects"].AsLong(1L);
		for ( long l = 0; l < connectsToDo; l++ ) {
			Trace("At connect Nr.: " << l);
			if ( ((connectsToDo - 1) == l) && data.IsDefined("LastChecksToDo") ) {
				data["ChecksToDo"] = data["LastChecksToDo"];
			}
			if ( cConfig["SSLConnector"].AsLong(1) == 1 ) {
				if ( cConfig["ConnectorToUse"].AsString() != "" ) {
					// deep clone, no side effect when adding Timeout
					Anything connectorConfig = GetTestCaseConfig()[cConfig["ConnectorToUse"].AsString()].DeepClone();
					if ( cConfig.IsDefined("TimeoutToUse") ) {
						connectorConfig["Timeout"] = cConfig["TimeoutToUse"].AsLong(0L);
					}
					if ( cConfig.IsDefined("UseThreadLocalMemory") ) {
						connectorConfig["UseThreadLocalMemory"] = cConfig["UseThreadLocalMemory"].AsLong(0L);
					}
					SSLConnector sc(connectorConfig);
					if ( cConfig["DoSendReceiveWithFailure"].AsLong(0) ) {
						DoSendReceiveWithFailure(&sc, data,
												 cConfig["IOSGoodAfterSend"].AsLong(0),
												 cConfig["IOSGoodBeforeSend"].AsLong(1));
					} else {
						DoSendReceive(&sc, data);
					}
				} else {
					SSLConnector sc("localhost", cConfig["PortToUse"].AsLong(0), cConfig["TimeoutToUse"].AsLong(0),
									(SSL_CTX *) NULL, (const char *) NULL, 0L, cConfig["UseThreadLocalMemory"].AsBool(0));

					if ( cConfig["DoSendReceiveWithFailure"].AsLong(0) ) {
						DoSendReceiveWithFailure(&sc, data,
												 cConfig["IOSGoodAfterSend"].AsLong(0),
												 cConfig["IOSGoodBeforeSend"].AsLong(1));
					} else {
						DoSendReceive(&sc, data);
					}
				}
			} else {
				Trace("Using configured NON SSL connector");
				Connector c("localhost", cConfig["PortToUse"].AsLong(0L), cConfig["TimeoutToUse"].AsLong(0L), String(), 0L, cConfig["UseThreadLocalMemory"].AsBool(0));
				if ( cConfig["DoSendReceiveWithFailure"].AsLong(0) ) {
					DoSendReceiveWithFailure(&c, data,
											 cConfig["IOSGoodAfterSend"].AsLong(0),
											 cConfig["IOSGoodBeforeSend"].AsLong(1));
				} else {
					DoSendReceive(&c, data);
				}
			}
		}
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *SSLListenerPoolTest::suite ()
{
	StartTrace(SSLListenerPoolTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SSLListenerPoolTest, PoolTest);
	ADD_CASE(testSuite, SSLListenerPoolTest, NullCallBackFactoryTest);
	ADD_CASE(testSuite, SSLListenerPoolTest, InitFailureNullAcceptorTest);
	return testSuite;
}
