/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "DateTimeTest.h"

//--- module under test --------------------------------------------------------
#include "DateTime.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------
#include <time.h>

//---- DateTimeTest ----------------------------------------------------------------
DateTimeTest::DateTimeTest(TString name)
	: ConfiguredTestCase(name, "DateTimeTestConfig")
{
	StartTrace(DateTimeTest.Ctor);
}

DateTimeTest::~DateTimeTest()
{
	StartTrace(DateTimeTest.Dtor);
}

// setup for this ConfiguredTestCase
void DateTimeTest::setUp ()
{
	StartTrace(DateTimeTest.setUp);
	ConfiguredTestCase::setUp();
}

void DateTimeTest::tearDown ()
{
	StartTrace(DateTimeTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void DateTimeTest::GetTimeZoneTest()
{
	StartTrace(DateTimeTest.test);
	t_assertm(timezone == DateTime::GetTimezone(), "expected timezone to be the same");
	Anything anyTimeUTC, anyTimeLocal;
	DateTime::GetTimeOfDay(anyTimeUTC, false);
	DateTime::GetTimeOfDay(anyTimeLocal, true);
	long lSecUTC = anyTimeUTC["sec_since_midnight"].AsLong(), lSecLoc = anyTimeLocal["sec_since_midnight"].AsLong(), lSecUTCOneMore = 0L;
	lSecUTC -= anyTimeUTC["tzone_sec"].AsLong();
	lSecUTCOneMore = (lSecUTC + 1L) % 86400;
	lSecUTC = lSecUTC % 86400;
	Trace("lSecUTC:" << lSecUTC << " lSecUTCOneMore:" << lSecUTCOneMore << " lSecLoc:" << lSecLoc);
	t_assertm( (lSecUTC == lSecLoc) || (lSecUTCOneMore == lSecLoc), "expected timezone correction to work and be within 1 second difference");
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *DateTimeTest::suite ()
{
	StartTrace(DateTimeTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, DateTimeTest, GetTimeZoneTest);

	return testSuite;
}
