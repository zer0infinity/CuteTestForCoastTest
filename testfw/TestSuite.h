/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef IT_TESTFW_TESTSUITE_H
#define IT_TESTFW_TESTSUITE_H

#include "Test.h"
class TestList;

//! <B>A TestSuite is a Composite of Tests</B>
/*!
 * A TestSuite is a Composite of Tests.
 * It runs a collection of test cases. Here is an example.
 *
 * TestSuite *suite= new TestSuite();
 * suite->addTest(new MathTest("testAdd"));
 * suite->addTest(new MathTest("testDivideByZero"));
 *
 * Note that TestSuites assume lifetime
 * control for any tests added to them.
 *
 * see @Test
 */
class TestSuite : public Test
{
public:
	TestSuite       ();
	~TestSuite		();

	void				run				(TestResult *result);
	int					countTestCases	();
	void				addTest			(Test *test);
	TString				toString		();
	Test 				*setClassName(const char *aName) {
		fClassName = aName;
		return this;
	}

private:
	TestList			*fTests;
};

// Returns a string representation of the test suite.
inline TString TestSuite::toString ()
{
	return this->getClassName ();
}

#define ADD_CASE(SUITE,TESTCASE,CASEMEMBER) \
			(SUITE)->addTest(NEW_CASE(TESTCASE,CASEMEMBER));

#endif
