/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnyBuiltInSortTest.h"
#include "DiffTimer.h"
#include <iostream>

bool AnyBuiltInSortTest::checksorted(const Anything &a, bool shouldfail) {
	for (long i = 0; i < a.GetSize() - 1; i++) {
		const char *s = a.SlotName(i);
		const char *t = a.SlotName(i + 1);
		if (strcmp(NotNull(s), NotNull(t)) > 0) {
			if (!shouldfail) {
				String msg("slots unsorted ");
				msg.Append(s).Append(" > ").Append(t);
				ASSERTM(msg.cstr(),false);
			}
			return shouldfail;
		}
	}
	return !shouldfail;
}
bool AnyBuiltInSortTest::checksortedbyvalue(const Anything &a, bool shouldfail) {
	for (long i = 0; i < a.GetSize() - 1; i++) {
		const char *s = a[i].AsCharPtr();
		const char *t = a[i + 1].AsCharPtr();
		if (strcmp(NotNull(s), NotNull(t)) > 0) {
			if (!shouldfail) {
				String msg("slots unsorted ");
				msg.Append(s).Append(" > ").Append(t);
				ASSERTM(msg.cstr(),false);
			}
			return shouldfail;
		}
	}
	return !shouldfail;
}
void AnyBuiltInSortTest::SortEmpty() {
	StartTrace(AnyBuiltInSortTest.SortEmpty);
	Anything a;
	Anything b(a);
	b.SortByKey();
	ASSERT_EQUAL(a, b);
}

void AnyBuiltInSortTest::SortOne() {
	StartTrace(AnyBuiltInSortTest.SortOne);
	Anything a("foo");
	Anything b(a);
	b.SortByKey();
	ASSERT_EQUAL(a, b);

}
void AnyBuiltInSortTest::SortTwo() {
	StartTrace(AnyBuiltInSortTest.SortTwo);
	Anything b;
	b["b"] = 1;
	b["a"] = 2;
	ASSERT_EQUAL(1, b[0].AsLong());
	ASSERT_EQUAL(2, b[1].AsLong());
	b.SortByKey();
	ASSERT(checksorted(b));
	ASSERT_EQUAL("a", b.SlotName(0));
	ASSERT_EQUAL("b", b.SlotName(1));
	ASSERT_EQUAL(1, b[1].AsLong());
	ASSERT_EQUAL(2, b[0].AsLong());
	ASSERT_EQUAL(2, b["a"].AsLong());
	ASSERT_EQUAL(1, b["b"].AsLong());
	b.SortByKey(); // should be same!
	ASSERT(checksorted(b));
	ASSERT_EQUAL("a", b.SlotName(0));
	ASSERT_EQUAL("b", b.SlotName(1));
	ASSERT_EQUAL(1, b[1].AsLong());
	ASSERT_EQUAL(2, b[0].AsLong());
	ASSERT_EQUAL(2, b["a"].AsLong());
	ASSERT_EQUAL(1, b["b"].AsLong());
}

void AnyBuiltInSortTest::SortThree() {
	StartTrace(AnyBuiltInSortTest.SortThree);
	Anything b;
	b["c"] = 1;
	b["a"] = 2;
	b["b"] = 3;
	ASSERT_EQUAL(1, b[0].AsLong());
	ASSERT_EQUAL(2, b[1].AsLong());
	ASSERT_EQUAL(3, b[2].AsLong());
	b.SortByKey();
	ASSERT(checksorted(b));
	b.SortByKey();
	ASSERT(checksorted(b));
}

void AnyBuiltInSortTest::SortMany() {
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
	ASSERT_EQUAL(size, a.GetSize());
	ASSERT_EQUAL(size, b.GetSize());
	ASSERTM("should be random",checksorted(a, true));
	DiffTimer dt;
	a.SortByKey();
	std::cerr << "sorting of " << size << " took " << dt.Diff() << " ms.\n";
	ASSERTM("should be sorted",checksorted(a));
	ASSERT_EQUAL(size, a.GetSize());
	for (i = 0; i < size; i++) {
		String m;
		m << "in slot i : " << i;
		ASSERT_EQUALM(m.cstr(),i, a[b.SlotName(i)].AsLong());
	}
	dt.Reset();
	a.SortByKey();
	std::cerr << "sorting of sorted " << size << " took " << dt.Diff() << " ms.\n";
	// now sort b by using the legacy SlotNameSorter...several orders of magnitude slower.
	dt.Reset();
	SlotnameSorter::Sort(b);
	std::cerr << "Slogname sorting of " << size << " took " << dt.Diff() << " ms.\n";
	ASSERT_EQUAL(a, b);
}

void AnyBuiltInSortTest::SortManyStringValues() {
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
		a.Append(sr);
		;
	}
	ASSERT_EQUAL(size, a.GetSize());
	ASSERTM("should be random",checksortedbyvalue(a, true));
	DiffTimer dt;
	a.SortByStringValues();
	std::cerr << "sorting of " << size << " took " << dt.Diff() << " ms.\n";
	ASSERTM("should be sorted",checksortedbyvalue(a));
	ASSERT_EQUAL(size, a.GetSize());
}

void AnyBuiltInSortTest::SortIsStable() {
	StartTrace(AnyBuiltInSortTest.SortIsStable);
	Anything a;
	a["d"] = 1;
	a["c"] = 2;
	a["a"] = 3;
	long i;
	for (i = 4; i < 10; i++) {
		a.Append(i);
	}
	ASSERTM("should be unsorted",checksorted(a, true));
	a.SortByKey();
	for (i = 0; i < a.GetSize() - 1; i++)
		if (!a.SlotName(i) && !a.SlotName(i + 1)) {
			String m;
			m << ("unstable at: ") << i << ":" << a[i].AsLong() << " not less than " << a[i+1].AsLong();
			ASSERTM(m.cstr(), a[i].AsLong() < a[i+1].AsLong());
		}
}

// builds up a suite of testcases, add a line for each testmethod
void AnyBuiltInSortTest::runAllTests(cute::suite &s) {
	StartTrace(AnyBuiltInSortTest.suite);
	s.push_back(CUTE_SMEMFUN(AnyBuiltInSortTest, SortEmpty));
	s.push_back(CUTE_SMEMFUN(AnyBuiltInSortTest, SortOne));
	s.push_back(CUTE_SMEMFUN(AnyBuiltInSortTest, SortTwo));
	s.push_back(CUTE_SMEMFUN(AnyBuiltInSortTest, SortThree));
	s.push_back(CUTE_SMEMFUN(AnyBuiltInSortTest, SortMany));
	s.push_back(CUTE_SMEMFUN(AnyBuiltInSortTest, SortManyStringValues));
	s.push_back(CUTE_SMEMFUN(AnyBuiltInSortTest, SortIsStable));
}
