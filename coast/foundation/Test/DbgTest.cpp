/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "DbgTest.h"

//--- module under test --------------------------------------------------------
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Anything.h"

//---- DbgTest ----------------------------------------------------------------
DbgTest::DbgTest(TString tname)
	: TestCase(tname)
{
}

DbgTest::~DbgTest()
{
}

void DbgTest::setUp ()
{
}

void DbgTest::tearDown ()
{
}

#ifdef DEBUG

void DbgTest::CheckTriggerTest()
{
	// load a debug context with all zero
	Tracer::Reset();

	// setup context with a range of main enable all 5L 10L
	Tracer::fgLowerBound = 5L;
	Tracer::fgUpperBound =  10L;
	Tracer::fgWDDebugContext["UpperBound"] = 10L;
	Tracer::fgWDDebugContext["LowerBound"] = 5L;

	// test the normal case, all main switches on and switch in range
	Tracer::fgWDDebugContext["DbgTest"]["MainSwitch"] = 5L;
	Tracer::fgWDDebugContext["DbgTest"]["FirstLevel"] = 5L;
	assertEqual(true, Tracer::CheckTrigger("DbgTest.FirstLevel"));

	// test the normal case second level
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["MainSwitch"] = 5L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["SecondLevel"] = 5L;
	assertEqual(0L, Tracer::fgWDDebugContext["DbgTest"]["EnableAll"].AsLong(1));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));

	// test the normal case third level
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["MainSwitch"] = 5L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["ThirdLevel"] = 5L;
	assertEqual(0L, Tracer::fgWDDebugContext["DbgTest"]["Second"]["EnableAll"].AsLong(1));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	// test main switch off at various levels
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["MainSwitch"] = 0L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	Tracer::fgWDDebugContext["DbgTest"]["Second"]["MainSwitch"] = 0L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));

	Tracer::fgWDDebugContext["DbgTest"]["MainSwitch"] = 0L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.FirstLevel"));

	// reset all main switches to on but set master switch off
	Tracer::fgWDDebugContext["LowerBound"] = 0L;
	Tracer::fgLowerBound = 0L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["MainSwitch"] = 5L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["MainSwitch"] = 5L;
	Tracer::fgWDDebugContext["DbgTest"]["MainSwitch"] = 5L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	// reset master switch to initial values and test out of range values
	// for the level switches, main switches are normal and on
	Tracer::fgWDDebugContext["LowerBound"] = 5L;
	Tracer::fgLowerBound = 5L;

	// underflow
	Tracer::fgWDDebugContext["DbgTest"]["FirstLevel"] = 4L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["SecondLevel"] = 4L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["ThirdLevel"] = 4L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	// overflow
	Tracer::fgWDDebugContext["DbgTest"]["FirstLevel"] = 11L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["SecondLevel"] = 11L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["ThirdLevel"] = 11L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	// reset level switches and test main switches out of range
	Tracer::fgWDDebugContext["DbgTest"]["FirstLevel"] = 5L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["SecondLevel"] = 5L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["ThirdLevel"] = 5L;

	// underflow
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["MainSwitch"] = 4L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["MainSwitch"] = 4L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	Tracer::fgWDDebugContext["DbgTest"]["MainSwitch"] = 4L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.FirstLevel"));

	// overflow
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["MainSwitch"] = 11L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["MainSwitch"] = 11L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	Tracer::fgWDDebugContext["DbgTest"]["MainSwitch"] = 11L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.FirstLevel"));

	// reset main switches and test enable all values
	Tracer::fgWDDebugContext["DbgTest"]["MainSwitch"] = 5L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["MainSwitch"] = 5L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["MainSwitch"] = 5L;

	// disable level switches
	Tracer::fgWDDebugContext["DbgTest"]["FirstLevel"] = 0L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["SecondLevel"] = 0L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["ThirdLevel"] = 0L;

	// enable all on the first level
	Tracer::fgWDDebugContext["DbgTest"]["EnableAll"] = 7L;
	assertEqual(true, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	// enable all on the second level
	Tracer::fgWDDebugContext["DbgTest"]["EnableAll"] = 0L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["EnableAll"] = 7L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	// enable all on the third level
	Tracer::fgWDDebugContext["DbgTest"]["EnableAll"] = 0L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["EnableAll"] = 0L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["EnableAll"] = 7L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	// enable all out of range overflow
	// enable all on the first level
	Tracer::fgWDDebugContext["DbgTest"]["EnableAll"] = 11L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["EnableAll"] = 0L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	// enable all out of range underflow
	// enable all on the first level
	Tracer::fgWDDebugContext["DbgTest"]["EnableAll"] = 4L;
	assertEqual(false, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	// check EnableAll with exclusion
	Tracer::fgWDDebugContext["DbgTest"]["EnableAll"] = 10L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["EnableAll"] = -1L;
	assertEqual(true, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["EnableAll"] = 7L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["MainSwitch"] = -1L;
	assertEqual(true, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	Tracer::fgWDDebugContext["DbgTest"]["Second"]["MainSwitch"] = -7L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["Third"]["MainSwitch"] = 1L;
	assertEqual(true, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));

	Tracer::fgWDDebugContext["DbgTest"]["Second"]["MainSwitch"] = 7L;
	Tracer::fgWDDebugContext["DbgTest"]["Second"]["SecondLevel"] = -7L;
	assertEqual(true, Tracer::CheckTrigger("DbgTest.FirstLevel"));
	assertEqual(false, Tracer::CheckTrigger("DbgTest.Second.SecondLevel"));
	assertEqual(true, Tracer::CheckTrigger("DbgTest.Second.Third.ThirdLevel"));
}

void DbgTest::CheckContextTriggerFailure()
{
	// load a debug context with all zero
	Tracer::Reset();

	// setup context with a range of main enable all 5L 10L
	Tracer::fgLowerBound = 30;
	Tracer::fgUpperBound =  50;
	Tracer::fgWDDebugContext["UpperBound"] = 50L;
	Tracer::fgWDDebugContext["LowerBound"] = 30L;

	assertEqual(true, Tracer::CheckTrigger("Context.HTMLDebug"));
}

void DbgTest::CheckTriggerTestFile()
{
	// load a debug context with all zero
	Tracer::Reset();

	// setup context with a range of main enable all 5L 10L
	Tracer::fgLowerBound = 550;
	Tracer::fgUpperBound =  650;
	Tracer::fgWDDebugContext["UpperBound"] = 650L;
	Tracer::fgWDDebugContext["LowerBound"] = 550L;

	assertEqual(false, Tracer::CheckTrigger("URLFilter.FilterState"));
	assertEqual(false, Tracer::CheckTrigger("URLFilter.DoFilterState"));
	assertEqual(true, Tracer::CheckTrigger("Server.FilterQuery"));
}

#endif

void DbgTest::CheckMacrosCompile()
{
	ResetTracer();
	StartTrace(DbgTest.CheckMacrosCompile);
	Trace("i was here");
	TraceBuf("buftrace so long", 10);
	Anything a;
	TraceAny(a, "an any");
	SubTrace("test", "a sub trace message");
	SubTraceAny("test", a, "a sub trace any");
	SubTraceBuf("test", "a subtrace buffer", 10);
	StatTrace("test", "a stat trace");
// couldn't test here TerminateTracer() and HTMLTraceStores()
}

Test *DbgTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;
#ifdef DEBUG
	testSuite->addTest (NEW_CASE(DbgTest, CheckTriggerTest));
	testSuite->addTest (NEW_CASE(DbgTest, CheckContextTriggerFailure));
	testSuite->addTest (NEW_CASE(DbgTest, CheckTriggerTestFile));
#endif
	testSuite->addTest (NEW_CASE(DbgTest, CheckMacrosCompile));
	return testSuite;

} // suite
