/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------

//--- interface include --------------------------------------------------------
#include "MailDATest.h"

//---- MailDATest ----------------------------------------------------------------
MailDATest::MailDATest(TString tname)
	: ConfiguredActionTest(tname, "MailDATestConfig")
{
	StartTrace(MailDATest.Ctor);
}

MailDATest::~MailDATest()
{
	StartTrace(MailDATest.Dtor);
}

// builds up a suite of testcases, add a line for each testmethod
Test *MailDATest::suite ()
{
	StartTrace(MailDATest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(MailDATest, RunTestCases));

	return testSuite;
} // suite
