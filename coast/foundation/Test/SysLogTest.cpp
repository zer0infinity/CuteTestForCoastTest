/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SysLogTest.h"

//--- module under test --------------------------------------------------------
#include "SysLog.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//---- SysLogTest ----------------------------------------------------------------
SysLogTest::SysLogTest(TString tname) : TestCaseType(tname)
{
	StartTrace(SysLogTest.Ctor);
}

SysLogTest::~SysLogTest()
{
	StartTrace(SysLogTest.Dtor);
}

void SysLogTest::setUp ()
{
	StartTrace(SysLogTest.setUp);
	::putenv("WD_LOGONCERR=1");
	SysLog::Init("SysLogTest");
}

void SysLogTest::tearDown ()
{
	StartTrace(SysLogTest.tearDown);
	SysLog::Terminate();
}

void SysLogTest::TestFlags ()
{
	StartTrace(SysLogTest.TestFlags);
	t_assertm(SysLog::fgSysLog != NULL, " expected creation of fgSysLog");
}

// builds up a suite of testcases, add a line for each testmethod
Test *SysLogTest::suite ()
{
	StartTrace(SysLogTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SysLogTest, TestFlags);

	return testSuite;
}
