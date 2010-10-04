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
#include "FoundationTestTypes.h"

//---- TimeStampTest ----------------------------------------------------------
class TimeStampTest : public TestFramework::TestCaseWithConfigAndStatistics
{
public:
	//! TestCase constructor
	//! \param name name of the test
	TimeStampTest(TString tstrName);

	//! destroys the test case
	~TimeStampTest();

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! describe this testcase
	void BasicOperatorsTest();
	void ArithmeticOperatorsTest();
	void AssignmentOperatorTest();
	void ConstructorsTest();
	void SetTimeTest();
	void InvalidTest();
	void RemoveNonNumericalCharsTest();
	void EmptyStringConstructorTest();
	void AsStringTest();
	void EmptyConstructorTest();
	void StaticMethodsTest();
	void ModifiersTest();
	void CtorTest();
	void WeekdayTest();
	void DayOfYearTest();
	void WeekOfYearTest();
	void MonthTest();
	void YearTest();
};

#endif
