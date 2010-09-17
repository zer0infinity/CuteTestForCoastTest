/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DbgTest_H
#define _DbgTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- DbgTest ----------------------------------------------------------
//!testcases for Tracer class
class DbgTest : public TestFramework::TestCase
{
public:
	//!constructors
	DbgTest(TString tstrName) : TestCaseType(tstrName) {}

	//!builds up a suite of testcases for this test
	static Test *suite ();

	void setUp();

	void tearDown();

	void DbgTestExplicitlyEnabled();
	void DbgTestLowerBoundZero();
	void DbgTestEnableAllFirstLevel();
	void DbgTestEnableAllSecondLevel();
	void DbgTestEnableAllThirdLevel();
	void DbgTestEnableAllAboveUpperBound();
	void DbgTestEnableAllBelowLowerBound();
	void DbgTestEnableAllSecondAndBelowDisabled();

	//!test case for the Dbg.h test macros
	void CheckMacrosCompile();
};

#endif
