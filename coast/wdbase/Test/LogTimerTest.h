/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LogTimerTest_H
#define _LogTimerTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- LogTimerTest ----------------------------------------------------------
//!TestCases description
class LogTimerTest : public TestFramework::TestCase
{
public:
	//!TestCase constructor
	//! \param name name of the test
	LogTimerTest(TString tstrName);

	//!destroys the test case
	~LogTimerTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!describe this testcase
	void MethodTimerTest();
};

#endif
