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

void SimpleListenerPoolTest::DoTestConnect()
{
	StartTrace(SimpleListenerPoolTest.DoTestConnect);
	{
		TraceAny(fConfig, "fConfig");
		Connector c("localhost", fConfig["TCP5010"]["Port"].AsLong(), 0L);
		DoSendReceive(&c, "5010_timeout_0");
	}
	{
		Connector c("localhost", fConfig["TCP5011"]["Port"].AsLong(), 0L);
		DoSendReceive(&c, "5011_timeout_0");
	}
	{
		Connector c("localhost", fConfig["TCP5012"]["Port"].AsLong(), 0L);
		DoSendReceive(&c, "5012_timeout_0");
	}
	{
		Connector c("localhost", fConfig["TCP5010"]["Port"].AsLong(), 1000L);
		DoSendReceive(&c, "5010_timeout_1000");
	}
	{
		Connector c("localhost", fConfig["TCP5011"]["Port"].AsLong(), 1000L);
		DoSendReceive(&c, "5011_timeout_1000");
	}
	{
		Connector c("localhost", fConfig["TCP5012"]["Port"].AsLong(), 1000L);
		DoSendReceive(&c, "5012_timeout_1000");
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
