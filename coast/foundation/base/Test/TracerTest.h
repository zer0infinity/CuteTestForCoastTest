/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TracerTest_H
#define _TracerTest_H

#include "TestCase.h"//lint !e537

class TracerTest : public testframework::TestCase
{
public:
	//!constructors
	TracerTest(TString tstrName) : TestCaseType(tstrName) {}

	//!builds up a suite of testcases for this test
	static Test *suite ();

	void setUp();

	void tearDown();

	void TracerTestExplicitlyEnabled();
	void TracerTestLowerBoundZero();
	void TracerTestEnableAllFirstLevel();
	void TracerTestEnableAllSecondLevel();
	void TracerTestEnableAllThirdLevel();
	void TracerTestEnableAllAboveUpperBound();
	void TracerTestEnableAllBelowLowerBound();
	void TracerTestEnableAllSecondAndBelowDisabled();
	void TracerTestNotAllAboveLowerBound();
	void TracerTestBug248();

	//!test case for the Tracer.h test macros
	void CheckMacrosCompile();
};

#endif
