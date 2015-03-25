/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MailDATest.h"
#include "TestSuite.h"


//---- MailDATest ----------------------------------------------------------------
MailDATest::MailDATest(TString tname)
	: ConfiguredActionTest(tname)
{
	StartTrace(MailDATest.MailDATest);
}

TString MailDATest::getConfigFileName()
{
	return "MailDATestConfig";
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
	ADD_CASE(testSuite, MailDATest, TestCases);
	return testSuite;
}
