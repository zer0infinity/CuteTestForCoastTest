/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "WPMStatHandlerTest.h"
#include "WPMStatHandler.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
bool WPMStatHandlerTest::AssertState(const WPMStatHandler &wpm, const Anything &state)
{
	return
		(assertEqualm(state[0L]["Value"].AsLong(0L), wpm.fPoolSize, state[0L]["Message"].AsCharPtr("failed")) ) 					&&
		(assertEqualm(state[1L]["Value"].AsLong(0L), wpm.fCurrentParallelRequests, state[1L]["Message"].AsCharPtr("failed")) ) 	&&
		(assertEqualm(state[2L]["Value"].AsLong(0L), wpm.fTotalRequests, state[2L]["Message"].AsCharPtr("failed")) )				&&
		(t_assertm(wpm.fTotalTime >= state[3L]["Value"].AsLong(0L), state[3L]["Message"].AsCharPtr("failed")) );
}

void WPMStatHandlerTest::ConstructorTest()
{
	StartTrace(WPMStatHandlerTest.ConstructorTest);
	Anything expectedState;
	expectedState[0L]["Value"] = 100;
	expectedState[0L]["Message"] = "fPoolSize wrong value";

	WPMStatHandler wpm(100);

	t_assertm(AssertState(wpm, expectedState), "wrong state");

}

void WPMStatHandlerTest::StatEvtTests()
{
	StartTrace(WPMStatHandlerTest.StatEvtTests);
	Anything expectedState;
	expectedState[0L]["Value"] = 100;
	expectedState[0L]["Message"] = "fPoolSize wrong value";
	expectedState[1L]["Value"] = 2;
	expectedState[1L]["Message"] = "fCurrentParallelRequests wrong value";
	expectedState[2L]["Value"] = 0;
	expectedState[2L]["Message"] = "fTotalRequests wrong value";
	expectedState[3L]["Value"] = 0;
	expectedState[3L]["Message"] = "fTotalTime wrong value";

	WPMStatHandler wpm(100);
	wpm.HandleStatEvt(WPMStatHandler::eEnter);
	wpm.HandleStatEvt(WPMStatHandler::eEnter);

	t_assertm(AssertState(wpm, expectedState), "wrong state");

	wpm.HandleStatEvt(WPMStatHandler::eLeave);

	expectedState[1L]["Value"] = 1;
	expectedState[1L]["Message"] = "fCurrentParallelRequests wrong value";
	expectedState[2L]["Value"] = 1;
	expectedState[2L]["Message"] = "fTotalRequests wrong value";
	expectedState[3L]["Value"] = 0;
	expectedState[3L]["Message"] = "fTotalTime wrong value";
	t_assertm(AssertState(wpm, expectedState), "wrong state");

	wpm.HandleStatEvt(WPMStatHandler::eLeave);
	expectedState[1L]["Value"] = 0;
	expectedState[1L]["Message"] = "fCurrentParallelRequests wrong value";
	expectedState[2L]["Value"] = 2;
	expectedState[2L]["Message"] = "fTotalRequests wrong value";
	expectedState[3L]["Value"] = 0;
	expectedState[3L]["Message"] = "fTotalTime wrong value";
	t_assertm(AssertState(wpm, expectedState), "wrong state");

}

void WPMStatHandlerTest::StatisticTests()
{
	StartTrace(WPMStatHandlerTest.StatEvtTests);
	Anything expectedState;
	expectedState[0L]["Value"] = 100;
	expectedState[0L]["Message"] = "fPoolSize wrong value";
	expectedState[1L]["Value"] = 2;
	expectedState[1L]["Message"] = "fCurrentParallelRequests wrong value";
	expectedState[2L]["Value"] = 0;
	expectedState[2L]["Message"] = "fTotalRequests wrong value";
	expectedState[3L]["Value"] = 0;
	expectedState[3L]["Message"] = "fTotalTime wrong value";

	WPMStatHandler wpm(100);
	wpm.HandleStatEvt(WPMStatHandler::eEnter);
	wpm.HandleStatEvt(WPMStatHandler::eEnter);

	Anything expected;
	Anything value;
	wpm.Statistic(value);
	value.Remove("TotalTime [ms]");
	expected["PoolSize"] = 100;
	expected["CurrentParallelRequests"] = 2;
	expected["MaxParallelRequests"] = 2;
	expected["TotalRequests"] = 0;
	expected["AverageTime [ms]"] = "0";
	expected["TRX/sec"] = "0";
	assertAnyEqual(expected, value);

	wpm.HandleStatEvt(WPMStatHandler::eLeave);
	wpm.HandleStatEvt(WPMStatHandler::eLeave);

	wpm.Statistic(value);
	value.Remove("TotalTime [ms]");
	value.Remove("AverageTime [ms]");
	value.Remove("TRX/sec");
	expected["PoolSize"] = 100;
	expected["CurrentParallelRequests"] = 0;
	expected["MaxParallelRequests"] = 2;
	expected["TotalRequests"] = 2;
	expected.Remove("AverageTime [ms]");
	expected.Remove("TRX/sec");
	assertAnyEqual(expected, value);
}

// builds up a suite of testcases, add a line for each testmethod
Test *WPMStatHandlerTest::suite ()
{
	StartTrace(WPMStatHandlerTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, WPMStatHandlerTest, ConstructorTest);
	ADD_CASE(testSuite, WPMStatHandlerTest, StatEvtTests);
	ADD_CASE(testSuite, WPMStatHandlerTest, StatisticTests);

	return testSuite;
}
