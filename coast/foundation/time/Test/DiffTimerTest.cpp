/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DiffTimerTest.h"
#include "TestSuite.h"
#include "DiffTimer.h"
#include "Tracer.h"
#include "SystemBase.h"

using namespace Coast;

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
	assertCompare(static_cast<DiffTimer::tTimeType>(100), less_equal, dt.TicksPerSecond());

	dt = DiffTimer((DiffTimer::eResolution)100);
	assertEqual(100L, dt.fResolution);
}

void DiffTimerTest::ScaleTest()
{
	StartTrace(DiffTimerTest.ScaleTest);
	DiffTimer dt;
	DiffTimer::tTimeType tps = DiffTimer::TicksPerSecond();
	double dFactor=static_cast<double>(tps)/DiffTimer::eMilliseconds;
	if ( dFactor >= 0.1 ) {
		// we can measure at least in milliseconds
		// assume at least a resolution of 10ms - linux default when using clock-ticks
		DiffTimer::tTimeType milliTime = 10;
		DiffTimer::tTimeType toScale = static_cast<DiffTimer::tTimeType>(dFactor*milliTime);
		assertCompare(milliTime, equal_to, dt.Scale(toScale, DiffTimer::eMilliseconds));
	} else {
		t_assertm(false, "need to invent some test method for resolutions less than 10ms");
	}
}

void DiffTimerTest::DiffTest()
{
	StartTrace(DiffTimerTest.DiffTest);

	DiffTimer::eResolution resolution( DiffTimer::eMilliseconds );
	DiffTimer dt(resolution); // milisecond resolution
	const HRTIME waitTime = 200L; //one fifth of a second, not only one twentieth
	t_assertm(System::MicroSleep(waitTime * resolution), "oops, interrupted"); // >= 50 miliseconds
	HRTIME result = dt.Diff() - waitTime;
	t_assertm( (result < 0 ? - result : result) <= waitTime / 5 , (const char *)(String("assume (+-20%) accuracy but was ") << static_cast<long>(result)));
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
