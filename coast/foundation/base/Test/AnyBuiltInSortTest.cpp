/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnyBuiltInSortTest.h"
#include "TestSuite.h"
#include "Anything.h"
#include "Dbg.h"
#include "DiffTimer.h"
#include <iostream>

//---- AnyBuiltInSortTest ----------------------------------------------------------------
AnyBuiltInSortTest::AnyBuiltInSortTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(AnyBuiltInSortTest.Ctor);
}

AnyBuiltInSortTest::~AnyBuiltInSortTest()
{
	StartTrace(AnyBuiltInSortTest.Dtor);
}

bool AnyBuiltInSortTest::checksorted(const Anything &a, bool shouldfail)
{
	for (long i = 0; i < a.GetSize() - 1; i++) {
		const char *s = a.SlotName(i);
		const char *t = a.SlotName(i + 1);
		if (strcmp(NotNull(s), NotNull(t)) > 0) {
			if (!shouldfail) {
				TString msg("slots unsorted ");
				msg.Append(s).Append(" > ").Append(t);
				t_assertm(false, msg);
			}
			return shouldfail;
		}
	}
	return !shouldfail;
}
bool AnyBuiltInSortTest::checksortedbyvalue(const Anything &a, bool shouldfail)
{
	for (long i = 0; i < a.GetSize() - 1; i++) {
		const char *s = a[i].AsCharPtr();
		const char *t = a[i+1].AsCharPtr();
		if (strcmp(NotNull(s), NotNull(t)) > 0) {
			if (!shouldfail) {
				TString msg("slots unsorted ");
				msg.Append(s).Append(" > ").Append(t);
				t_assertm(false, msg);
			}
			return shouldfail;
		}
	}
	return !shouldfail;
}
void AnyBuiltInSortTest::SortEmpty()
{
	StartTrace(AnyBuiltInSortTest.SortEmpty);
	Anything a;
	Anything b(a);
	b.SortByKey();
	assertAnyEqual(a, b);
}

void AnyBuiltInSortTest::SortOne()
{
	StartTrace(AnyBuiltInSortTest.SortOne);
	Anything a("foo");
	Anything b(a);
	b.SortByKey();
	assertAnyEqual(a, b);

}
void AnyBuiltInSortTest::SortTwo()
{
	StartTrace(AnyBuiltInSortTest.SortTwo);
	Anything b;
	b["b"] = 1;
	b["a"] = 2;
	assertEqual(1, b[0].AsLong());
	assertEqual(2, b[1].AsLong());
	b.SortByKey();
	t_assert(checksorted(b));
	assertEqual("a", b.SlotName(0));
	assertEqual("b", b.SlotName(1));
	assertEqual(1, b[1].AsLong());
	assertEqual(2, b[0].AsLong());
	assertEqual(2, b["a"].AsLong());
	assertEqual(1, b["b"].AsLong());
	b.SortByKey(); // should be same!
	t_assert(checksorted(b));
	assertEqual("a", b.SlotName(0));
	assertEqual("b", b.SlotName(1));
	assertEqual(1, b[1].AsLong());
	assertEqual(2, b[0].AsLong());
	assertEqual(2, b["a"].AsLong());
	assertEqual(1, b["b"].AsLong());
}

void AnyBuiltInSortTest::SortThree()
{
	StartTrace(AnyBuiltInSortTest.SortThree);
	Anything b;
	b["c"] = 1;
	b["a"] = 2;
	b["b"] = 3;
	assertEqual(1, b[0].AsLong());
	assertEqual(2, b[1].AsLong());
	assertEqual(3, b[2].AsLong());
	b.SortByKey();
	t_assert(checksorted(b));
	b.SortByKey();
	t_assert(checksorted(b));
}

void AnyBuiltInSortTest::SortMany()
{
	StartTrace(AnyBuiltInSortTest.SortMany);
	Anything a;
	Anything b;
	const long size = 1000;
	long i;
	for (i = 0; i < size; i++) {
#if defined(WIN32)
		long r = rand();
#else
		long r = random();
#endif
		String sr;
		sr << i << "bar" << r << "foo"; // needs i to make it unique
		a[sr] = i;
		b[sr] = i;
	}
	assertEqual(size, a.GetSize());
	assertEqual(size, b.GetSize());
	t_assertm(checksorted(a, true), "should be random");
	DiffTimer dt;
	a.SortByKey();
	std::cerr << "sorting of " << size << " took " << dt.Diff() << " ms.\n";
	t_assertm(checksorted(a), "should be sorted");
	assertEqual(size, a.GetSize());
	for (i = 0; i < size; i++) {
		if (!assertEqualm(i, a[b.SlotName(i)].AsLong(), TString("in slot i : ") << i)) {
			break;
		}
	}
	dt.Reset();
	a.SortByKey();
	std::cerr << "sorting of sorted " << size << " took " << dt.Diff() << " ms.\n";
	// now sort b by using the legacy SlotNameSorter...several orders of magnitude slower.
	dt.Reset();
	SlotnameSorter::Sort(b);
	std::cerr << "Slogname sorting of " << size << " took " << dt.Diff() << " ms.\n";
	assertAnyEqual(a, b);
}

void AnyBuiltInSortTest::SortManyStringValues()
{
	StartTrace(AnyBuiltInSortTest.SortManyStringValues);
	Anything a;
	Anything b;
	const long size = 1000;
	for (long i = 0; i < size; i++) {
#if defined(WIN32)
		long r = rand();
#else
		long r = random();
#endif
		String sr;
		sr << "bar" << r << "foo"; // needs i to make it unique
		a.Append(sr);;
	}
	assertEqual(size, a.GetSize());
	t_assertm(checksortedbyvalue(a, true), "should be random");
	DiffTimer dt;
	a.SortByStringValues();
	std::cerr << "sorting of " << size << " took " << dt.Diff() << " ms.\n";
	t_assertm(checksortedbyvalue(a), "should be sorted");
	assertEqual(size, a.GetSize());
}

void AnyBuiltInSortTest::SortIsStable()
{
	StartTrace(AnyBuiltInSortTest.SortIsStable);
	Anything a;
	a["d"] = 1;
	a["c"] = 2;
	a["a"] = 3;
	long i;
	for (i = 4; i < 10; i++) {
		a.Append(i);
	}
	t_assertm(checksorted(a, true), "should be unsorted");
	a.SortByKey();
	for (i = 0; i < a.GetSize() - 1; i++)
		if (!a.SlotName(i) && !a.SlotName(i + 1)) {
			t_assertm(a[i].AsLong() < a[i+1].AsLong(), TString("unstable at: ") << i << ":" << a[i].AsLong() << " not less than " << a[i+1].AsLong());
		}

}

// builds up a suite of testcases, add a line for each testmethod
Test *AnyBuiltInSortTest::suite ()
{
	StartTrace(AnyBuiltInSortTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnyBuiltInSortTest, SortEmpty);
	ADD_CASE(testSuite, AnyBuiltInSortTest, SortOne);
	ADD_CASE(testSuite, AnyBuiltInSortTest, SortTwo);
	ADD_CASE(testSuite, AnyBuiltInSortTest, SortThree);
	ADD_CASE(testSuite, AnyBuiltInSortTest, SortMany);
	ADD_CASE(testSuite, AnyBuiltInSortTest, SortManyStringValues);
	ADD_CASE(testSuite, AnyBuiltInSortTest, SortIsStable);
	return testSuite;
}
