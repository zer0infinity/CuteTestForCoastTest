/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef SYSTEMBASETEST_H_
#define SYSTEMBASETEST_H_

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- SystemBaseTest ----------------------------------------------------------
//!testcases for SystemBase
class SystemBaseTest : public TestFramework::TestCase
{
public:
	//!constructors
	SystemBaseTest(TString tstrName);
	~SystemBaseTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	void GetProcessEnvironmentTest();

	void LockFileTest();

	void DoSingleSelectTest();
	void MicroSleepTest();

	//!storage tests
	void allocFreeTests();

	void TimeTest();
};

#endif
