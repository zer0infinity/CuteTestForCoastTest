/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//--- standard modules used ----------------------------------------------------
#include "ITOString.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"
//--- module under test --------------------------------------------------------
#include "SysLog.h"

//--- interface include --------------------------------------------------------
#include "SysLogTest.h"

//---- SysLogTest ----------------------------------------------------------------
SysLogTest::SysLogTest(TString tname) : TestCase(tname)
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

	testSuite->addTest (NEW_CASE(SysLogTest, TestFlags));

	return testSuite;
} // suite
