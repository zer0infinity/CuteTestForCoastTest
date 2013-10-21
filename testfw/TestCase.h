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
#include "TestResult.h"

//---- ##Class## ----------------------------------------------------------
//! A TestCase defines the fixture to run multiple tests
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
 class MathTest : public testframework::TestCase {
     protected: int m_value1;
     protected: int m_value2;
     public: MathTest (TString tstrName)
                 : TestCaseType(tstrName) {
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
    protected: void AddTest() {
        int result = value1 + value2;
        cu_assert (result == 5);
    }
</PRE>
 *
 The following code creates an instance of MathTest which
 executes only the testAdd method:
<PRE>
 Test *test = NEW_CASE(MathTest, AddTest);
 test.run ();

 or the following to add a test to a TestSuite:

 ADD_CASE(testSuite, MathTest, AddTest);
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

namespace testframework
{

	template
	<
	class dummy
	>
	class NoStatisticPolicy
	{
		NoStatisticPolicy(const NoStatisticPolicy &);
	public:
		typedef NoStatisticPolicy<dummy> StatisticPolicyType;

		NoStatisticPolicy()	{}

		~NoStatisticPolicy() {}

		class CatchTime
		{
		public:
			CatchTime(TString, StatisticPolicyType *) {}
			~CatchTime() {}
		};
		typedef typename StatisticPolicyType::CatchTime CatchTimeType;

	protected:
		void ExportCsvStatistics() {}
		void AddStatisticOutput(TString, long) {}
		void LoadData(TString, TString) {}
		void StoreData() {}
	};

	template
	<
	class dummy
	>
	class NoConfigPolicy
	{
		NoConfigPolicy(const NoConfigPolicy &);
	public:
		typedef NoConfigPolicy<dummy> ConfigPolicyType;

		NoConfigPolicy() {}

		virtual ~NoConfigPolicy() {}

	protected:
		bool loadConfig(TString, TString) {
			return true;
		}
		void unloadConfig() {}
		virtual TString getConfigFileName() {
			return "";
		}
	};

	template
	<
	template <typename> class ConfigPolicy = NoConfigPolicy,
			 template <typename> class StatisticPolicy = NoStatisticPolicy,
			 class dummy = int
			 >
	class TestCaseT
		: public Test
		, public StatisticPolicy<dummy>
		, public ConfigPolicy<dummy>
	{
		TestCaseT();
		TestCaseT(const TestCaseT &);
	public:
		typedef TestCaseT<ConfigPolicy, StatisticPolicy, dummy> TestCaseType;
		typedef void (TestCaseType::*CaseMemberPtr)();

		typedef StatisticPolicy<dummy> StatisticPolicyType;
		typedef ConfigPolicy<dummy> ConfigPolicyType;
		typedef typename StatisticPolicyType::CatchTime CatchTimeType;

		TestCaseT(TString tname)
			: fName(tname)
		{}
		virtual ~TestCaseT()
		{}

		virtual void run(TestResult *result) {
			Test::run(result);
			result->startTest (this);

			StatisticPolicyType::LoadData(getClassName(), name());
			t_assertm( ConfigPolicyType::loadConfig(getClassName(), name()), TString("expected ") << ConfigPolicyType::getConfigFileName() << " to be readable!" );

			setUp();
			doRunTest();
			tearDown();

			ConfigPolicyType::unloadConfig();
			StatisticPolicyType::StoreData();

			result->endTest(this);
		}

		virtual TestResult *run() {
			TestResult *result = new TestResult;
			run (result);
			return result;
		}

		virtual int countTestCases() {
			return 1;
		}

		TString name() {
			return fName;
		}

		TString	toString() {
			return TString (this->getClassName()) << "." << name();
		}

		TestCaseType *setTheTest(CaseMemberPtr p_TheTest) {
			fTheTest = p_TheTest;
			return this;
		}

	protected:
		virtual void setUp() {}

		virtual void tearDown() {}

		virtual void doRunTest() {
			(this->*fTheTest)();
		}

	protected:
		const TString	fName;
		CaseMemberPtr	fTheTest;
	};

	typedef TestCaseT<NoConfigPolicy, NoStatisticPolicy, int> TestCase;

#define NEW_CASE(TESTCASE,CASEMEMBER)	(TESTCASE *)(((new TESTCASE(#CASEMEMBER))->setTheTest( (TESTCASE::CaseMemberPtr)&TESTCASE::CASEMEMBER) )->setClassName(#TESTCASE))

}	// end namespace testframework

#endif
