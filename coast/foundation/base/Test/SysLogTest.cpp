/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SysLogTest.h"
#include "SystemLog.h"
#include "TestSuite.h"
#include "Tracer.h"

SysLogTest::SysLogTest(TString tname) :
		TestCaseType(tname) {
	StartTrace(SysLogTest.Ctor);
}

SysLogTest::~SysLogTest() {
	StartTrace(SysLogTest.Dtor);
}

void SysLogTest::setUp() {
	StartTrace(SysLogTest.setUp);
	SystemLog::Init("SysLogTest");
}

void SysLogTest::tearDown() {
	StartTrace(SysLogTest.tearDown);
	SystemLog::Terminate();
}

void SysLogTest::TestFlags() {
	StartTrace(SysLogTest.TestFlags);
	t_assertm(SystemLog::getSysLog() != NULL, " expected creation of fgSysLog");
}

// builds up a suite of testcases, add a line for each testmethod
Test *SysLogTest::suite() {
	StartTrace(SysLogTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SysLogTest, TestFlags);
	return testSuite;
}
