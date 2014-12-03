/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef SYSTEMBASETEST_H_
#define SYSTEMBASETEST_H_

#include "FoundationTestTypes.h"

class SystemBaseTest : public testframework::TestCase
{
public:
	SystemBaseTest(TString tstrName);

	//!builds up a suite of testcases for this test
	static Test *suite ();

	void GetProcessEnvironmentTest();

#if !defined(WIN32)
	void LockFileTest();
#endif

	void DoSingleSelectTest();
	void MicroSleepTest();

	//!storage tests
	void allocFreeTests();

	void TimeTest();

	void SnPrintf_ReturnsBytesOfContentWrittenWithoutTerminatingZero();
	void SnPrintf_ReturnsBytesRequiredWithoutTerminatingZero();
	void SnPrintf_WritesTerminatingZeroEvenWhenTruncatingBuffer();
};

#endif
