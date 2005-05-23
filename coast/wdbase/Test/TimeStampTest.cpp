/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TimeStampTest.h"

//--- module under test --------------------------------------------------------
#include "TimeStamp.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "System.h"
#include "DiffTimer.h"
#include "SysLog.h"

//--- c-modules used -----------------------------------------------------------
#include <limits.h>

//---- TimeStampTest ----------------------------------------------------------------
TimeStampTest::TimeStampTest(TString tstrName)
	: ConfiguredTestCase(tstrName, "TimeStampTestConfig")
{
	StartTrace(TimeStampTest.Ctor);
}

TimeStampTest::~TimeStampTest()
{
	StartTrace(TimeStampTest.Dtor);
}

// setup for this TestCase
void TimeStampTest::setUp ()
{
	StartTrace(TimeStampTest.setUp);
	ConfiguredTestCase::setUp();
}

void TimeStampTest::tearDown ()
{
	StartTrace(TimeStampTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void TimeStampTest::testBasicOperators()
{
	StartTrace(TimeStampTest.testBasicOperators);
	TraceAny(fTestCaseConfig, "Tests config");
	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		ROAnything cConfig = fTestCaseConfig[i];
		TraceAny(cConfig, "current config");
		long upper(cConfig["NumberOfRuns"].AsLong(1));
		for ( long l = 0; l < upper; l++ ) {
			Trace("At testindex: " << i);
			TimeStamp ts1(cConfig["Date"].AsString());
			TimeStamp ts2(cConfig["Date"].AsString());
			TimeStamp tsAfter(cConfig["DateAfter"].AsString());
			TimeStamp tsBefore(cConfig["DateBefore"].AsString());
			TimeStamp dateAddResult(cConfig["DateAddResult"].AsString());

			//operator ==
			t_assertm(ts1 == ts2, TString("Expected == test to pass at testindex:") << i);
			t_assertm(ts1.IsEqual(ts2), TString("Expected == test to pass at testindex:") << i);
			t_assertm(!(ts1 == tsAfter), TString("Expected == test to pass at testindex:") << i);
			t_assertm(ts1.IsEqual(tsAfter) == false, TString("Expected == test to pass at testindex:") << i);

			//operator !=
			t_assertm(tsAfter != tsBefore, TString("Expected != test to pass at testindex:") << i);
			t_assertm(tsAfter.IsNotEqual(tsBefore), TString("Expected != test to pass at testindex:") << i);
			t_assertm(!(ts1 != ts1), TString("Expected != test to pass at testindex:") << i);
			t_assertm(ts1.IsNotEqual(ts1) == false, TString("Expected != test to pass at testindex:") << i);

			//operator >
			t_assertm(tsAfter > ts2, TString("Expected > test to pass at testindex:") << i);
			t_assertm(tsAfter.IsAfter(ts1), TString("Expected > test to pass at testindex:") << i);
			t_assertm(!(tsBefore > tsAfter), TString("Expected > test to pass at testindex:") << i);
			t_assertm(tsBefore.IsAfter(tsAfter) == false, TString("Expected > test to pass at testindex:") << i);
			t_assertm(!(tsAfter > tsAfter), TString("Expected > boundary test to pass at testindex:") << i);

			//operator <
			t_assertm(tsBefore < ts2, TString("Expected < test to pass at testindex:") << i);
			t_assertm(tsBefore.IsBefore(ts1), TString("Expected < test to pass at testindex:") << i);
			t_assertm(!(tsAfter < tsBefore), TString("Expected < test to pass at testindex:") << i);
			t_assertm(tsAfter.IsBefore(tsBefore) == false, TString("Expected < test to pass at testindex:") << i);
			t_assertm(!(tsBefore < tsBefore), TString("Expected < boundary test to pass at testindex:") << i);

			//operator >=
			t_assertm(tsAfter >= ts2, TString("Expected >= test to pass at testindex:") << i);
			t_assertm(tsAfter.IsAfterEqual(ts1), TString("Expected >= test to pass at testindex:") << i);
			t_assertm(!(tsBefore >= tsAfter), TString("Expected >= test to pass at testindex:") << i);
			t_assertm(tsBefore.IsAfterEqual(tsAfter) == false, TString("Expected >= test to pass at testindex:") << i);
			t_assertm((tsAfter >= tsAfter), TString("Expected >= boundary test to pass at testindex:") << i);

			//operator <=
			t_assertm(tsBefore <= ts2, TString("Expected <= test to pass at testindex:") << i);
			t_assertm(tsBefore.IsBeforeEqual(ts1), TString("Expected <= test to pass at testindex:") << i);
			t_assertm(!(tsAfter <= tsBefore), TString("Expected <= test to pass at testindex:") << i);
			t_assertm(tsAfter.IsBeforeEqual(tsBefore) == false, TString("Expected <= test to pass at testindex:") << i);
			t_assertm((tsBefore <= tsBefore), TString("Expected <= boundary test to pass at testindex:") << i);

			//combined expression
			t_assertm( ( ts1 + cConfig["LongToAdd"].AsLong() ) >= dateAddResult, TString("Expected combined expressiont test to pass at testindex:") << i);
			t_assertm( ( ts1 + cConfig["LongToAdd"].AsLong() ) <= dateAddResult, TString("Expected combined expressiont test to pass at testindex:") << i);
			t_assertm(!( ( ts1 + cConfig["LongToAdd"].AsLong() ) <  dateAddResult ), TString("Expected combined expressiont test to pass at testindex:") << i);
			t_assertm(!( ( ts1 + cConfig["LongToAdd"].AsLong() ) >  dateAddResult ), TString("Expected combined expressiont test to pass at testindex:") << i);
			t_assertm(  ts1 == ts2, TString("Expected ts1 not to have changed at testindex:") << i);
		}
	}
}

void TimeStampTest::testArithmeticOperators()
{
	StartTrace(TimeStampTest.testArithmeticOperators);
	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		ROAnything cConfig = fTestCaseConfig[i];
		Trace("At testindex: " << i);
		long upper(cConfig["NumberOfRuns"].AsLong(1));
		for ( long l = 0; l < upper; l ++ ) {
			TimeStamp ts1(cConfig["Date"].AsString());
			TimeStamp result;
			result = ts1 + cConfig["AmountSec"].AsLong();
			t_assertm(result.AsString() == cConfig["ExpectedResult"].AsString(), "Expected + test to pass");
			Trace("result after operator+ is: " << result.AsString());

			result = result - cConfig["AmountSec"].AsLong();
			t_assertm(result.AsString() == cConfig["Date"].AsString(), "Expected - test to pass");
			Trace("result after operator- is: " << result.AsString());
		}
	}
}

void TimeStampTest::testConstructors()
{
	StartTrace(TimeStampTest.testArithmeticOperators);
	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		ROAnything cConfig = fTestCaseConfig[i];
		Trace("At testindex: " << i);
		TimeStamp result(cConfig["ExpectedResult"].AsString());
		// Check result is ok.
		t_assertm(cConfig["ExpectedResult"].AsString() == result.AsString(), "Expected result initialization test to pass");

		TimeStamp copyConstTs(cConfig["ExpectedResult"].AsString());
		t_assertm(copyConstTs == result, "Expected copy constructor test to pass");
		t_assertm(cConfig["ExpectedResult"].AsString() == copyConstTs.AsString(), "Expected copy constructor test to pass");

		TimeStamp setTs;
		setTs.Set(cConfig["ExpectedResult"].AsString());
		t_assertm(setTs == result, "Expected Set() test to pass");
		t_assertm(cConfig["ExpectedResult"].AsString() == setTs.AsString(), "Expected copy constructor test to pass");
	}
}

void TimeStampTest::testEmptyStringConstructor()
{
	StartTrace(TimeStampTest.testEmptyStringConstructor);
	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		ROAnything cConfig = fTestCaseConfig[i];
		Trace("At testindex: " << i);
		TimeStamp ts("");
		t_assertm(cConfig["ExpectedResult"].AsString() == ts.AsString(), "Expected constructor with empty string test to pass");
		Trace(ts.AsString());
		TimeStamp tsTwo;
		tsTwo.Set("");
		t_assertm(cConfig["ExpectedResult"].AsString() == tsTwo.AsString(), "Expected set with empty string test to pass");
	}
}

void TimeStampTest::testEmptyConstructor()
{
	StartTrace(TimeStampTest.testEmptyStringConstructor);
	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		ROAnything cConfig = fTestCaseConfig[i];
		Trace("At testindex: " << i);
		TimeStamp tsNow = TimeStamp::Now();
		TimeStamp tsDefConstructor;
		t_assertm(tsNow <= tsDefConstructor, "Expected empty constructor to generate timestamp with Now date.");
		Trace("tsNow: " << tsNow.AsString() << " tsDefConstructor: " << tsDefConstructor.AsString());
		// Generates 19700101000000
		TimeStamp ts2(0);
		t_assertm(cConfig["ExpectedResult"].AsString() == ts2.AsString(), "Expected set with empty string test to pass");
		Trace(ts2.AsString());
	}
}

void TimeStampTest::testRemoveNonNumericalChars()
{
	StartTrace(TimeStampTest.testRemoveNonNumericalChars);
	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		ROAnything cConfig = fTestCaseConfig[i];
		Trace("At testindex: " << i);
		TimeStamp taintedOne(cConfig["DateTainted"].AsString());
		assertEqualm(cConfig["DateUntainted"].AsCharPtr(), taintedOne.AsString(), TString("Expected RemoveNonNumericalChars test to pass at index ") << i);
		Trace("untainted: " << taintedOne.AsString());
		Trace("expeced: " << cConfig["DateUntainted"].AsString());
		TimeStamp taintedTwo;
		taintedTwo.Set(cConfig["DateTainted"].AsString());
		assertEqualm(cConfig["DateUntainted"].AsCharPtr(), taintedTwo.AsString(), TString("Expected RemoveNonNumericalChars test to pass at index ") << i);
		Trace("untainted: " << taintedOne.AsString());
	}
}

void TimeStampTest::testAssignmentOperator()
{
	StartTrace(TimeStampTest.testAssignmentOperator);
	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		ROAnything cConfig = fTestCaseConfig[i];
		Trace("At testindex: " << i);
		TimeStamp one(cConfig["DateOne"].AsString());
		t_assertm(cConfig["DateOne"].AsString() == one.AsString(), "Expected result initialization test to pass");

		TimeStamp two(cConfig["DateTwo"].AsString());
		t_assertm(cConfig["DateTwo"].AsString() == two.AsString(), "Expected result initialization test to pass");

		TimeStamp three(cConfig["DateThree"].AsString());
		t_assertm(cConfig["DateThree"].AsString() == three.AsString(), "Expected result initialization test to pass");

		one = two = three;

		t_assertm(cConfig["DateThree"].AsString() == one.AsString(), "Expected assignment test to pass");
		t_assertm(cConfig["DateThree"].AsString() == two.AsString(), "Expected assignment test to pass");
		t_assertm(cConfig["DateThree"].AsString() == three.AsString(), "Expected assignment test to pass");
	}
}

void TimeStampTest::testAsString()
{
	StartTrace(TimeStampTest.testAsString);
	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		ROAnything cConfig = fTestCaseConfig[i];
		Trace("At testindex: " << i);
		TimeStamp ts(cConfig["Date"].AsString());
		t_assertm(cConfig["Date"].AsString() == ts.AsString(), "Expected AsString test to pass");
		t_assertm(cConfig["DateWithZ"].AsString() == ts.AsStringWithZ(), "Expected AsStringWithZ test to pass");
		Trace("The String with Z: " << ts);
	}
}

void TimeStampTest::testInvalid()
{
	StartTrace(TimeStampTest.testInvalid);
	TimeStamp result(-1);
	t_assertm(!result.IsValid(), "Expected invalid value");
	assertEqualm("19691231235959", result.AsString(), "Expected result not valid");
	result.Set("20000102030405");
	t_assertm(result.IsValid(), "Expected valid value");
	assertEqual(946782245, result.AsLong());

	TimeStamp tsMax(TimeStamp::Max()), tsMin(TimeStamp::Min());
	t_assertm(tsMax.IsValid(), "Expected valid value");
	t_assertm(tsMin.IsValid(), "Expected valid value");
	tsMin -= 1;
	tsMax += 1;
	t_assertm(!tsMax.IsValid(), "Expected invalid value");
	t_assertm(!tsMin.IsValid(), "Expected invalid value");
	assertEqualm("19691231235959", tsMax.AsString(), "Date must be invalid now");
	assertEqualm("19691231235959", tsMin.AsString(), "Date must be invalid now");
}

void TimeStampTest::testStaticMethods()
{
	StartTrace(TimeStampTest.testStaticMethods);
	{
		t_assertm("20371231235959Z" == TimeStamp::Max().AsStringWithZ(), "Expected max test to pass");
		t_assertm("20371231235959" == TimeStamp::Max().AsString(), "Expected max test to pass");
	}
	{
		t_assertm("19700101000000Z" == TimeStamp::Min().AsStringWithZ(), "Expected max test to pass");
		t_assertm("19700101000000" == TimeStamp::Min().AsString(), "Expected max test to pass");
	}
}

void TimeStampTest::ModifiersTest()
{
	StartTrace(TimeStampTest.ModifiersTest);
	TimeStamp aInitial(0), aSecond(0);
	t_assert(aInitial.IsValid());
	t_assert(aSecond.IsValid());
	assertEqual(0, aInitial.AsLong());
	assertEqual(0, aInitial.fTime);
	assertEqual("19700101000000", aInitial.AsString());
	assertEqual("", aInitial.fRep);
	t_assert(aSecond == aInitial);
	aSecond += 1200;
	assertEqual(1200, aSecond.fTime);
	assertEqual(1200, aSecond.AsLong());
	t_assert(aSecond.IsValid());
	aInitial = aSecond;
	assertEqual(1200, aInitial.AsLong());
	assertEqual(1200, aInitial.fTime);
	t_assert(aInitial.IsValid());
	assertEqual("19700101002000", aInitial.AsString());
	aSecond -= 1200;
	assertEqual(0, aSecond.AsLong());
	t_assert(aSecond.IsValid());
	assertEqual("19700101000000", aSecond.AsString());
	aSecond -= 1200;
	// underflow! keep same value but mark as bad
	t_assert(!aSecond.IsValid());
	assertEqual(0, aSecond.AsLong());
	// not self healing...
	t_assert(!aSecond.IsValid());

	aSecond = 1200;
	t_assert(aSecond.IsValid());
	assertEqual(1200, aSecond.fTime);
	assertEqual(1200, aSecond.AsLong());
	assertEqual("19700101002000", aSecond.AsString());
	aSecond = -1200;
	t_assert(!aSecond.IsValid());
	assertEqual(0, aSecond.fTime);
	assertEqual(0, aSecond.AsLong());
	assertEqual("19691231235959", aSecond.AsString());

	aSecond = "20000102030405";
	t_assert(aSecond.IsValid());
	assertEqual(946782245, aSecond.fTime);
	assertEqual(946782245, aSecond.AsLong());

	TimeStamp aThird(aSecond);
	t_assert( aSecond == aThird );

	// synthetic test
	aSecond.fTime = 1234;
	aSecond.fRep = "5678";
	aSecond.fTimeSet = true;
	assertEqual(1234, aSecond.AsLong());
	assertEqual("5678", aSecond.AsString());

	// copy ctor does not correct wrong things, just copies
	TimeStamp aFourth(aSecond);
	assertEqual(aSecond.fTime, aFourth.fTime);
	assertEqual(aSecond.fTimeSet, aFourth.fTimeSet);
	assertEqual(aSecond.fRep, aFourth.fRep);
	assertEqual(aSecond.AsLong(), aFourth.AsLong());
	assertEqual(aSecond.AsString(), aFourth.AsString());

	aSecond.fTimeSet = false;
	// this one still works
	assertEqual("5678", aSecond.AsString());
	aSecond.fRep.Trim(0);
	assertEqual("19691231235959", aSecond.AsString());
}

void TimeStampTest::CtorTest()
{
	StartTrace(TimeStampTest.CtorTest);
	{
		TimeStamp aStamp(19, 70, 1, 1, 0, 0, 1);
		assertCharPtrEqual("19700101000001", aStamp.AsString());
		assertEqual(1L, aStamp.AsLong());
	}
	{
		TimeStamp aStamp(19, 69, 1, 1, 0, 0, 1);
		assertCharPtrEqual("19691231235959", aStamp.AsString());
		assertEqual(0L, aStamp.AsLong());
	}
	{
		TimeStamp aStamp(20, 4, 2, 29, 0, 0, 1), aStamp2("20040229000001");
		assertCharPtrEqual("20040229000001", aStamp.AsString());
		t_assert(aStamp == aStamp2);
	}
}

void TimeStampTest::WeekdayTest()
{
	StartTrace(TimeStampTest.WeekDayTest);
	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		ROAnything cConfig = fTestCaseConfig[i];
		Trace("At testindex: " << i);
		TimeStamp ts(cConfig["Date"].AsString());
		Trace("Date [" << ts.AsString() << "]");
		assertEqualm(cConfig["Weekday"].AsLong(-1), ts.Weekday(), "Expected same weekday number");
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *TimeStampTest::suite ()
{
	StartTrace(TimeStampTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, TimeStampTest, testBasicOperators);
	ADD_CASE(testSuite, TimeStampTest, testAssignmentOperator);
	ADD_CASE(testSuite, TimeStampTest, testArithmeticOperators);
	ADD_CASE(testSuite, TimeStampTest, testConstructors);
	ADD_CASE(testSuite, TimeStampTest, testInvalid);
	ADD_CASE(testSuite, TimeStampTest, testRemoveNonNumericalChars);
	ADD_CASE(testSuite, TimeStampTest, testEmptyStringConstructor);
	ADD_CASE(testSuite, TimeStampTest, testAsString);
	ADD_CASE(testSuite, TimeStampTest, testEmptyConstructor);
	ADD_CASE(testSuite, TimeStampTest, testStaticMethods);
	ADD_CASE(testSuite, TimeStampTest, ModifiersTest);
	ADD_CASE(testSuite, TimeStampTest, CtorTest);
	ADD_CASE(testSuite, TimeStampTest, WeekdayTest);
	return testSuite;
}
