/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Socket.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "RequestListener.h"

//--- interface include --------------------------------------------------------
#include "SimpleListenerPoolTest.h"

//---- SimpleListenerPoolTest ----------------------------------------------------------------
SimpleListenerPoolTest::SimpleListenerPoolTest(TString tname) : ListenerPoolTest(tname)
{
	StartTrace(SimpleListenerPoolTest.Ctor);
}

SimpleListenerPoolTest::~SimpleListenerPoolTest()
{
	StartTrace(SimpleListenerPoolTest.Dtor);
}

void SimpleListenerPoolTest::PoolTest()
{
	StartTrace(SimpleListenerPoolTest.PoolTest);

	Anything config;
	config.Append("TCP5010");
	config.Append("TCP5011");
	config.Append("TCP5012");
	config.Append("TCP5013");
	config.Append("TCP5014");
	config.Append("TCP5015");

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
	Anything failures = tcbf->GetFailures();
	t_assertm(failures.GetSize() == 0, "Receivers encountered a least one error");
	if (failures.GetSize()) {
		TraceAny(failures, "EYECATCHER Receivers encountered the following errors");
	}
}

void SimpleListenerPoolTest::DoTestConnect()
{
	StartTrace(SimpleListenerPoolTest.DoTestConnect);

	FOREACH_ENTRY("SimpleListenerPoolTest", cConfig, cName) {
		TraceAny(cConfig, "cConfig");

		Connector c("localhost",
					cConfig["PortToUse"].AsLong(0L),
					cConfig["TimeoutToUse"].AsLong(0L));
		if ( cConfig["DoSendReceiveWithFailure"].AsLong(0) ) {
			DoSendReceiveWithFailure(&c, cConfig["Data"].DeepClone(), cConfig["IOSGoodAfterSend"].AsLong(0));
		} else {
			DoSendReceive(&c, cConfig["Data"].DeepClone());
		}
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *SimpleListenerPoolTest::suite ()
{
	StartTrace(SimpleListenerPoolTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(SimpleListenerPoolTest, PoolTest));
#if !defined(WIN32)
	testSuite->addTest (NEW_CASE(SimpleListenerPoolTest, InitFailureTest));	// test does not work on WIN32
#endif
	testSuite->addTest (NEW_CASE(SimpleListenerPoolTest, NullCallBackFactoryTest));
	testSuite->addTest (NEW_CASE(SimpleListenerPoolTest, InitFailureNullAcceptorTest));

	return testSuite;

} // suite
