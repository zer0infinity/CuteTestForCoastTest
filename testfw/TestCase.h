/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef IT_TESTFW_TESTCASE_H
#define IT_TESTFW_TESTCASE_H

#include "Test.h"

class TestCase;

typedef void (TestCase::* CaseMemberPtr)();

// testcase *tst;
//	NEW_CASE(&tst, ..., TEST1, FOO);

#define NEW_CASE(TESTCASE,CASEMEMBER) \
	 (TESTCASE *)(((new TESTCASE(#CASEMEMBER))->setTheTest((CaseMemberPtr)\
				    &TESTCASE##::##CASEMEMBER))->setClassName(#TESTCASE))

//---- ##Class## ----------------------------------------------------------
//! <B>A TestCase defines the fixture to run multiple tests</B>
/*!
 A test case defines the fixture to run multiple tests. To define a test case
 1) implement a subclass of TestCase
 2) define instance variables that store the state of the fixture
 3) initialize the fixture state by overriding setUp
 4) clean-up after a test by overriding tearDown.
 *
 Each test runs in its own fixture so there
 can be no side effects among test runs.
 Here is an example:
<PRE>
 class MathTest : public TestCase {
     protected: int m_value1;
     protected: int m_value2;
     public: MathTest (TString tstrName)
                 : TestCase (tstrName) {
     }
     protected: void setUp () {
         m_value1 = 2;
         m_value2 = 3;
     }
 }
</PRE>

 For each test implement a method which interacts
 with the fixture. Verify the expected results with assertions specified
 by calling cu_assert on the expression you want to test:
<PRE>
    protected: void testAdd () {
        int result = value1 + value2;
        cu_assert (result == 5);
    }
</PRE>
 *
 The following code creates an instance of MathTest which
 executes only the testAdd method:
<PRE>
 Test *test = NEW_CASE(MathTest, testAdd);
 test.run ();

 or the following to add a test to a TestSuite:

 ADD_CASE(testSuite, MathTest, testAdd);
</PRE>

 The tests to be run can be collected into a TestSuite. CppUnit provides
 different test runners which can run a test suite and collect the results.
 The test runners expect a static method suite as the entry
 point to get a test to run.
<PRE>
 public: static MathTest::suite () {
      TestSuite *suite = new TestSuite;
      ADD_CASE(suite,MathTest,testAdd);
      ADD_CASE(suite,MathTest,testDivideByZero);
      return suiteOfTests;
  }
</PRE>
 Note that the caller of suite assumes lifetime control
 for the returned suite.
 *
 see @TestResult and @TestSuite.
 *
 */
class TestCase : public Test
{
protected:
	const TString		fName;
	CaseMemberPtr fTheTest;

public:
	TestCase		(TString Name);
	virtual				~TestCase		();

	virtual void		run				(TestResult *result);
	virtual TestResult  *run			();
	virtual int			countTestCases	();
	TString				name			();
	TString				toString		();
	TestCase 			*setTheTest(CaseMemberPtr p_TheTest);
	void thisTest() {}

protected:
	virtual void		setUp			();
	virtual void		tearDown		();
	virtual void		runTest			();

};

inline TestCase::TestCase (TString tname) : fName (tname)
{}

inline TestCase::~TestCase ()
{}

inline int TestCase::countTestCases ()
{
	return 1;
}

inline TString TestCase::name ()
{
	return fName;
}

inline void TestCase::setUp ()
{}

inline void TestCase::tearDown ()
{}

inline TString TestCase::toString ()
{
	return TString (this->getClassName ()) << "." << name ();
}

#endif
