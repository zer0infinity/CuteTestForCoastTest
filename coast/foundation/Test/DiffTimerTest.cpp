/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "DiffTimer.h"

//--- interface include --------------------------------------------------------
#include "DiffTimerTest.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "System.h"

//--- c-library modules used ---------------------------------------------------

//---- DiffTimerTest ----------------------------------------------------------------
DiffTimerTest::DiffTimerTest(TString tname) : TestCaseType(tname)
{
}

DiffTimerTest::~DiffTimerTest()
{
}

Test *DiffTimerTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, DiffTimerTest, ConstructorTest);
	ADD_CASE(testSuite, DiffTimerTest, ScaleTest);
	ADD_CASE(testSuite, DiffTimerTest, DiffTest);
	ADD_CASE(testSuite, DiffTimerTest, SimulatedDiffTest);
	return testSuite;

}

void DiffTimerTest::ConstructorTest()
{
	StartTrace(DiffTimerTest.ConstructorTest);

	DiffTimer dt;

	assertEqual(1000L, dt.fResolution);
	t_assert(dt.TicksPerSecond() >= 100);

	dt = DiffTimer((DiffTimer::eResolution)100);
	assertEqual(100L, dt.fResolution);
}

void DiffTimerTest::ScaleTest()
{
	StartTrace(DiffTimerTest.ScaleTest);

	DiffTimer dt;
#if defined(__linux__)
	assertEqual(100L, dt.Scale(10, 1000));
#endif

#if defined(__sun)
	assertEqual(10000L, dt.Scale(10 * dt.TicksPerSecond(), DiffTimer::eMilliseconds));
	assertEqual(10L, dt.Scale(10 * 1000000, DiffTimer::eMilliseconds));
#endif

#if defined(WIN32)
	assertEqual(10, dt.Scale(10, 1000));
#endif
}

void DiffTimerTest::DiffTest()
{
	StartTrace(DiffTimerTest.DiffTest);

	DiffTimer::eResolution resolution( DiffTimer::eMilliseconds );
	DiffTimer dt(resolution); // milisecond resolution
	const HRTIME wait = 200L; //one fifth of a second, not only one twentieth
	t_assertm(System::MicroSleep(wait * resolution), "oops, interrupted"); // >= 50 miliseconds
	HRTIME result = dt.Diff() - wait;
	t_assertm( (result < 0 ? result * -1 : result) <= wait / 5 , (const char *)(String("assume (+-20%) accuracy but was ") << result));
	dt = DiffTimer(DiffTimer::eSeconds);
	t_assertm(System::MicroSleep(600 * resolution), "oops, interrupted"); // >= 50 miliseconds
	result = dt.Diff();
	t_assertm(result == 0L, "expected no round to take effect");
	dt = DiffTimer(DiffTimer::eSeconds);
	t_assertm(System::MicroSleep(1100 * resolution), "oops, interrupted"); // >= 50 miliseconds
	result = dt.Diff();
	t_assertm(result == 1L, "expected no round to take effect");
}

void DiffTimerTest::SimulatedDiffTest()
{
	StartTrace(DiffTimerTest.SimulatedDiffTest);

	DiffTimer dt;
	assertEqual(3000, dt.Diff(3000));
}
