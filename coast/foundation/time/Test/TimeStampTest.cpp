/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TimeStampTest.h"
#include "TimeStamp.h"
#include "TestSuite.h"

void TimeStampTest::BasicOperatorsTest() {
	StartTrace(TimeStampTest.BasicOperatorsTest);

	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		long upper(roaConfig["NumberOfRuns"].AsLong(1));
		for (long l = 0; l < upper; ++l) {
			TimeStamp ts1(roaConfig["Date"].AsString());
			TimeStamp ts2(roaConfig["Date"].AsString());
			TimeStamp tsAfter(roaConfig["DateAfter"].AsString());
			TimeStamp tsBefore(roaConfig["DateBefore"].AsString());
			TimeStamp dateAddResult(roaConfig["DateAddResult"].AsString());

			//operator ==
			t_assertm(ts1 == ts2, TString("Expected == test to pass at ") << strCase);
			t_assertm(ts1.IsEqual(ts2), TString("Expected == test to pass at ") << strCase);
			t_assertm(!(ts1 == tsAfter), TString("Expected == test to pass at ") << strCase);
			t_assertm(ts1.IsEqual(tsAfter) == false, TString("Expected == test to pass at ") << strCase);

			//operator !=
			t_assertm(tsAfter != tsBefore, TString("Expected != test to pass at ") << strCase);
			t_assertm(tsAfter.IsNotEqual(tsBefore), TString("Expected != test to pass at ") << strCase);
			t_assertm(!(ts1 != ts1), TString("Expected != test to pass at ") << strCase);
			t_assertm(ts1.IsNotEqual(ts1) == false, TString("Expected != test to pass at ") << strCase);

			//operator >
			t_assertm(tsAfter > ts2, TString("Expected > test to pass at ") << strCase);
			t_assertm(tsAfter.IsAfter(ts1), TString("Expected > test to pass at ") << strCase);
			t_assertm(!(tsBefore > tsAfter), TString("Expected > test to pass at ") << strCase);
			t_assertm(tsBefore.IsAfter(tsAfter) == false, TString("Expected > test to pass at ") << strCase);
			t_assertm(!(tsAfter > tsAfter), TString("Expected > boundary test to pass at ") << strCase);

			//operator <
			t_assertm(tsBefore < ts2, TString("Expected < test to pass at ") << strCase);
			t_assertm(tsBefore.IsBefore(ts1), TString("Expected < test to pass at ") << strCase);
			t_assertm(!(tsAfter < tsBefore), TString("Expected < test to pass at ") << strCase);
			t_assertm(tsAfter.IsBefore(tsBefore) == false, TString("Expected < test to pass at ") << strCase);
			t_assertm(!(tsBefore < tsBefore), TString("Expected < boundary test to pass at ") << strCase);

			//operator >=
			t_assertm(tsAfter >= ts2, TString("Expected >= test to pass at ") << strCase);
			t_assertm(tsAfter.IsAfterEqual(ts1), TString("Expected >= test to pass at ") << strCase);
			t_assertm(!(tsBefore >= tsAfter), TString("Expected >= test to pass at ") << strCase);
			t_assertm(tsBefore.IsAfterEqual(tsAfter) == false, TString("Expected >= test to pass at ") << strCase);
			t_assertm((tsAfter >= tsAfter), TString("Expected >= boundary test to pass at ") << strCase);

			//operator <=
			t_assertm(tsBefore <= ts2, TString("Expected <= test to pass at ") << strCase);
			t_assertm(tsBefore.IsBeforeEqual(ts1), TString("Expected <= test to pass at ") << strCase);
			t_assertm(!(tsAfter <= tsBefore), TString("Expected <= test to pass at ") << strCase);
			t_assertm(tsAfter.IsBeforeEqual(tsBefore) == false, TString("Expected <= test to pass at ") << strCase);
			t_assertm((tsBefore <= tsBefore), TString("Expected <= boundary test to pass at ") << strCase);

			//combined expression
			t_assertm( ( ts1 + roaConfig["LongToAdd"].AsLong() ) >= dateAddResult, TString("Expected combined expressiont test to pass at ") << strCase);
			t_assertm( ( ts1 + roaConfig["LongToAdd"].AsLong() ) <= dateAddResult, TString("Expected combined expressiont test to pass at ") << strCase);
			t_assertm(!( ( ts1 + roaConfig["LongToAdd"].AsLong() ) < dateAddResult ), TString("Expected combined expressiont test to pass at ") << strCase);
			t_assertm(!( ( ts1 + roaConfig["LongToAdd"].AsLong() ) > dateAddResult ), TString("Expected combined expressiont test to pass at ") << strCase);
			t_assertm( ts1 == ts2, TString("Expected ts1 not to have changed at ") << strCase);
		}
	}
}

void TimeStampTest::ArithmeticOperatorsTest() {
	StartTrace(TimeStampTest.ArithmeticOperatorsTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		long upper(roaConfig["NumberOfRuns"].AsLong(1));
		for (long l = 0; l < upper; ++l) {
			TimeStamp ts1(roaConfig["Date"].AsString());
			TimeStamp result;
			result = ts1 + roaConfig["AmountSec"].AsLong();
			assertCharPtrEqualm(roaConfig["ExpectedResult"].AsString(), result.AsString(), (TString("Expected + test to pass at ") << strCase));
			Trace("result after operator+ is: " << result.AsString());

			result = result - roaConfig["AmountSec"].AsLong();
			assertCharPtrEqualm(roaConfig["Date"].AsString(), result.AsString(), (TString("Expected - test to pass at ") << strCase));
			Trace("result after operator- is: " << result.AsString());
		}
	}
}

void TimeStampTest::ConstructorsTest() {
	StartTrace(TimeStampTest.ArithmeticOperatorsTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp result(roaConfig["ExpectedResult"].AsString());
		// Check result is ok.
		t_assertm(roaConfig["ExpectedResult"].AsString() == result.AsString(), TString("Expected result initialization test to pass at ") << strCase);

		TimeStamp copyConstTs(roaConfig["ExpectedResult"].AsString());
		t_assertm(copyConstTs == result, TString("Expected copy constructor test to pass at ") << strCase);
		t_assertm(roaConfig["ExpectedResult"].AsString() == copyConstTs.AsString(), TString("Expected copy constructor test to pass at ") << strCase);

		TimeStamp setTs;
		setTs.Set(roaConfig["ExpectedResult"].AsString());
		t_assertm(setTs == result, TString("Expected Set() test to pass at ") << strCase);
		t_assertm(roaConfig["ExpectedResult"].AsString() == setTs.AsString(), TString("Expected copy constructor test to pass at ") << strCase);
	}
}

void TimeStampTest::EmptyStringConstructorTest() {
	StartTrace(TimeStampTest.EmptyStringConstructorTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp ts("");
		t_assertm(roaConfig["ExpectedResult"].AsString() == ts.AsString(), TString("Expected constructor with empty string test to pass at ") << strCase);
		Trace(ts.AsString());
		TimeStamp tsTwo;
		tsTwo.Set("");
		t_assertm(roaConfig["ExpectedResult"].AsString() == tsTwo.AsString(), TString("Expected set with empty string test to pass at ") << strCase);
	}
}

void TimeStampTest::EmptyConstructorTest() {
	StartTrace(TimeStampTest.EmptyStringConstructorTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp tsNow = TimeStamp::Now();
		TimeStamp tsDefConstructor;
		t_assertm(tsNow <= tsDefConstructor, TString("Expected empty constructor to generate timestamp with Now date. at ") << strCase);
		Trace("tsNow: " << tsNow.AsString() << " tsDefConstructor: " << tsDefConstructor.AsString());
		// Generates 19700101000000
		TimeStamp ts2((time_t) 0);
		t_assertm(roaConfig["ExpectedResult"].AsString() == ts2.AsString(), TString("Expected set with empty string test to pass at ") << strCase);
		Trace(ts2.AsString());
	}
}

void TimeStampTest::RemoveNonNumericalCharsTest() {
	StartTrace(TimeStampTest.RemoveNonNumericalCharsTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp taintedOne(roaConfig["DateTainted"].AsString());
		assertEqualm(roaConfig["DateUntainted"].AsCharPtr(), taintedOne.AsString(), TString("Expected RemoveNonNumericalChars test to pass at ") << strCase);
		Trace("untainted: " << taintedOne.AsString());
		Trace("expeced: " << roaConfig["DateUntainted"].AsString());
		TimeStamp taintedTwo;
		taintedTwo.Set(roaConfig["DateTainted"].AsString());
		assertEqualm(roaConfig["DateUntainted"].AsCharPtr(), taintedTwo.AsString(), TString("Expected RemoveNonNumericalChars test to pass at ") << strCase);
		Trace("untainted: " << taintedOne.AsString());
	}
}

void TimeStampTest::AssignmentOperatorTest() {
	StartTrace(TimeStampTest.AssignmentOperatorTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp one(roaConfig["DateOne"].AsString());
		t_assertm(roaConfig["DateOne"].AsString() == one.AsString(), TString("Expected result initialization test to pass at ") << strCase);

		TimeStamp two(roaConfig["DateTwo"].AsString());
		t_assertm(roaConfig["DateTwo"].AsString() == two.AsString(), TString("Expected result initialization test to pass at ") << strCase);

		TimeStamp three(roaConfig["DateThree"].AsString());
		t_assertm(roaConfig["DateThree"].AsString() == three.AsString(), TString("Expected result initialization test to pass at ") << strCase);

		one = two = three;

		t_assertm(roaConfig["DateThree"].AsString() == one.AsString(), TString("Expected assignment test to pass at ") << strCase);
		t_assertm(roaConfig["DateThree"].AsString() == two.AsString(), TString("Expected assignment test to pass at ") << strCase);
		t_assertm(roaConfig["DateThree"].AsString() == three.AsString(), TString("Expected assignment test to pass at ") << strCase);
	}
}

void TimeStampTest::AsStringTest() {
	StartTrace(TimeStampTest.AsStringTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp ts(roaConfig["Date"].AsString());
		t_assertm(roaConfig["Date"].AsString() == ts.AsString(), TString("Expected AsString test to pass at ") << strCase);
		t_assertm(roaConfig["DateWithZ"].AsString() == ts.AsStringWithZ(), TString("Expected AsStringWithZ test to pass at ") << strCase);
	}
}

void TimeStampTest::InvalidTest() {
	StartTrace(TimeStampTest.InvalidTest);
	TimeStamp result(-1);
	t_assertm(!result.IsValid(), "Expected invalid value");
	assertEqualm("", result.AsString(), "Expected result not valid");
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
	assertEqualm("", tsMax.AsString(), "Date must be invalid now");
	assertEqualm("", tsMin.AsString(), "Date must be invalid now");
}

void TimeStampTest::StaticMethodsTest() {
	StartTrace(TimeStampTest.StaticMethodsTest);
	{
		t_assertm("20371231235959Z" == TimeStamp::Max().AsStringWithZ(), "Expected max test to pass");
		t_assertm("20371231235959" == TimeStamp::Max().AsString(), "Expected max test to pass");
	}
	{
		t_assertm("19700101000000Z" == TimeStamp::Min().AsStringWithZ(), "Expected max test to pass");
		t_assertm("19700101000000" == TimeStamp::Min().AsString(), "Expected max test to pass");
	}
}

void TimeStampTest::ModifiersTest() {
	StartTrace(TimeStampTest.ModifiersTest);
	TimeStamp aInitial((time_t) 0), aSecond((time_t) 0);
	t_assert(aInitial.IsValid());
	t_assert(aSecond.IsValid());
	assertEqual(0, aInitial.AsLong());
	assertEqual("19700101000000", aInitial.AsString());
	t_assert(aSecond == aInitial);
	aSecond += 1200;
	assertEqual(1200, aSecond.AsLong());
	t_assert(aSecond.IsValid());
	aInitial = aSecond;
	assertEqual(1200, aInitial.AsLong());
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
	assertEqual(1200, aSecond.AsLong());
	assertEqual("19700101002000", aSecond.AsString());
	aSecond = -1200;
	t_assert(!aSecond.IsValid());
	assertEqual(0, aSecond.AsLong());
	assertEqual("", aSecond.AsString());

	aSecond = "20000102030405";
	t_assert(aSecond.IsValid());
	assertEqual(946782245, aSecond.AsLong());

	TimeStamp aThird(aSecond);
	t_assert( aSecond == aThird );

	// copy ctor does not correct wrong things, just copies
	TimeStamp aFourth(aSecond);
	assertEqual(aSecond.AsLong(), aFourth.AsLong());
	assertEqual(aSecond.AsString(), aFourth.AsString());
}

void TimeStampTest::CtorTest() {
	StartTrace(TimeStampTest.CtorTest);
	{
		TimeStamp aStamp(19, 70, 1, 1, 0, 0, 1);
		assertCharPtrEqual("19700101000001", aStamp.AsString());
		assertEqual(1L, aStamp.AsLong());
	}
	{
		TimeStamp aStamp(19, 69, 1, 1, 0, 0, 1);
		assertCharPtrEqual("", aStamp.AsString());
		assertEqual(0L, aStamp.AsLong());
	}
	{
		TimeStamp aStamp(20, 4, 2, 29, 0, 0, 1), aStamp2("20040229000001");
		assertCharPtrEqual("20040229000001", aStamp.AsString());
		t_assert(aStamp == aStamp2);
	}
	{
		TimeStamp aStamp("XXX");
		t_assert(!aStamp.IsValid());
		assertCharPtrEqual("", aStamp.AsString());
	}
	Trace("sizeof TimeStamp:" << (long)sizeof(TimeStamp));
	Trace("sizeof TimeStamp::intTimeRep:" << (long)sizeof(TimeStamp::intTimeRep));
}

void TimeStampTest::SetTimeTest() {
	StartTrace(TimeStampTest.SetTimeTest);
	{
		TimeStamp aStamp(19, 70, 1, 1, 0, 0, 1);
		assertCharPtrEqual("19700101000001", aStamp.AsString());
		assertEqual(1L, aStamp.AsLong());
		t_assert( aStamp.SetTime(12, 12, 12) );
		assertCharPtrEqual("19700101121212", aStamp.AsString());
		t_assert( aStamp.SetTime(23, 59, 59) );
		assertCharPtrEqual("19700101235959", aStamp.AsString());
		t_assert( !aStamp.SetTime(24, 59, 59) );
		assertCharPtrEqual("19700101235959", aStamp.AsString());
		t_assert( !aStamp.SetTime(23, 60, 59) );
		t_assert( !aStamp.SetTime(23, 59, 60) );
		t_assert( aStamp.SetTime(0, 0, 0) );
		assertCharPtrEqual("19700101000000", aStamp.AsString());
	}
}

void TimeStampTest::WeekdayTest() {
	StartTrace(TimeStampTest.WeekDayTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp ts(roaConfig["Date"].AsString());
		Trace("Date [" << ts.AsString() << "]");
		assertEqualm(roaConfig["Weekday"].AsLong(-1), ts.Weekday(), TString("Expected same weekday number at ") << strCase);
	}
}

void TimeStampTest::DayOfYearTest() {
	StartTrace(TimeStampTest.DayOfYearTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp ts(roaConfig["Date"].AsString());
		Trace("Date [" << ts.AsString() << "]");
		assertEqualm(roaConfig["DayOfYear"].AsLong(-1), ts.DayOfYear(), TString("Expected same day of year at ") << strCase);
	}
}

void TimeStampTest::WeekOfYearTest() {
	StartTrace(TimeStampTest.WeekOfYearTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp ts(roaConfig["Date"].AsString());
		assertEqualm(roaConfig["WeekOfYear"].AsLong(-1), ts.WeekOfYear(), TString("Expected same week of year [") << roaConfig["Date"].AsString() << "] at " << strCase);
	}
}

void TimeStampTest::MonthTest() {
	StartTrace(TimeStampTest.MonthTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp::AMonth::eMonthNumber aMonthNum((TimeStamp::AMonth::eMonthNumber) roaConfig["Month"].AsLong());
		TimeStamp::AMonth aM((aMonthNum));
		assertComparem( (TimeStamp::TSIntNumberType)roaConfig["ExpectedSeconds"].AsLong(-1L), equal_to, aM.AsSeconds( roaConfig["InYear"].AsLong(1970) ), TString("Expected same seconds for mnum:") << roaConfig["Month"].AsLong() << " inyear:" << roaConfig["InYear"].AsLong(1970) << " at " << strCase);
	}
}

void TimeStampTest::YearTest() {
	StartTrace(TimeStampTest.YearTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TString strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase << "idx:" << aEntryIterator.Index();
		}

		TraceAny(roaConfig, "current config");
		TimeStamp::AYear aY(roaConfig["Year"].AsLong());
		assertComparem( (TimeStamp::TSIntNumberType)roaConfig["ExpectedSeconds"].AsLong(-1L), equal_to, aY.AsSeconds(), TString("Expected same seconds at ") << strCase);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *TimeStampTest::suite() {
	StartTrace(TimeStampTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, TimeStampTest, BasicOperatorsTest);
	ADD_CASE(testSuite, TimeStampTest, AssignmentOperatorTest);
	ADD_CASE(testSuite, TimeStampTest, ArithmeticOperatorsTest);
	ADD_CASE(testSuite, TimeStampTest, ConstructorsTest);
	ADD_CASE(testSuite, TimeStampTest, SetTimeTest);
	ADD_CASE(testSuite, TimeStampTest, InvalidTest);
	ADD_CASE(testSuite, TimeStampTest, RemoveNonNumericalCharsTest);
	ADD_CASE(testSuite, TimeStampTest, EmptyStringConstructorTest);
	ADD_CASE(testSuite, TimeStampTest, AsStringTest);
	ADD_CASE(testSuite, TimeStampTest, EmptyConstructorTest);
	ADD_CASE(testSuite, TimeStampTest, StaticMethodsTest);
	ADD_CASE(testSuite, TimeStampTest, ModifiersTest);
	ADD_CASE(testSuite, TimeStampTest, CtorTest);
	ADD_CASE(testSuite, TimeStampTest, WeekdayTest);
	ADD_CASE(testSuite, TimeStampTest, DayOfYearTest);
	ADD_CASE(testSuite, TimeStampTest, WeekOfYearTest);
	ADD_CASE(testSuite, TimeStampTest, MonthTest);
	ADD_CASE(testSuite, TimeStampTest, YearTest);
	return testSuite;
}
