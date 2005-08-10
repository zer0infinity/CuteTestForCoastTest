/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TimeStampTest_H
#define _TimeStampTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "Anything.h"

//---- TimeStampTest ----------------------------------------------------------
class TimeStampTest : public TestCase
{
	Anything fConfig;
	ROAnything fTestCaseConfig;

public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	TimeStampTest(TString tstrName);

	//! destroys the test case
	~TimeStampTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! describe this testcase
	void testBasicOperators();
	void testArithmeticOperators();
	void testAssignmentOperator();
	void testConstructors();
	void testInvalid();
	void testRemoveNonNumericalChars();
	void testEmptyStringConstructor();
	void testAsString();
	void testEmptyConstructor();
	void testStaticMethods();
	void ModifiersTest();
	void CtorTest();
	void WeekdayTest();
	void DayOfYearTest();
	void WeekOfYearTest();
};

#endif
