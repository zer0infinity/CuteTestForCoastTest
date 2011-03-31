/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DateTimeTest.h"
#include "DateTime.h"
#include "TestSuite.h"
#include "Dbg.h"
#include <time.h>

//---- DateTimeTest ----------------------------------------------------------------
DateTimeTest::DateTimeTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(DateTimeTest.DateTimeTest);
}

TString DateTimeTest::getConfigFileName()
{
	return "DateTimeTestConfig";
}

DateTimeTest::~DateTimeTest()
{
	StartTrace(DateTimeTest.Dtor);
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

// builds up a suite of tests, add a line for each testmethod
Test *DateTimeTest::suite ()
{
	StartTrace(DateTimeTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, DateTimeTest, GetTimeZoneTest);

	return testSuite;
}
