/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnythingIteratorTest.h"

void AnythingIteratorTest::testEmptyAnythingBegin() {
	Anything a;
	ASSERT(a.begin() == a.end());
	const Anything &b = a;
	ASSERT(b.begin() == b.end());
}
void AnythingIteratorTest::testSimpleAnythingBegin() {
	Anything a(1L);
	ASSERT(a.begin() != a.end());
	const Anything &b = a;
	ASSERT(b.begin() != b.end());
}
void AnythingIteratorTest::testSimpleAnythingDeref() {
	Anything a(1L);
	ASSERT(*a.begin() == a);
	const Anything &b = a;
	ASSERT(*b.begin() == b);
}
void AnythingIteratorTest::testSimpleAnythingIncrement() {
	Anything a(1L);
	Anything_iterator ai = a.begin();
	ASSERT(++ai == a.end());
	ASSERT(ai != a.begin());
	const Anything &b = a;
	Anything_const_iterator bi = b.begin();
	ASSERT(++bi == b.end());
	ASSERT(bi != b.begin());
}
void AnythingIteratorTest::testSimpleAnythingDecrement() {
	Anything a(1L);
	Anything_iterator ai = a.end();
	ASSERT(--ai == a.begin());
	ASSERT(ai != a.end());
	const Anything &b = a;
	Anything_const_iterator bi = b.end();
	ASSERT(--bi == b.begin());
	ASSERT(bi != b.end());
}
void AnythingIteratorTest::testSimpleAnythingAssignment() {
	Anything a(1L);
	*a.begin() = 5L;
	ASSERT(*a.begin() == Anything(5L));

}
void AnythingIteratorTest::testAnythingIteration() {
	Anything a;
	a.Append(1L);
	a.Append(2L);
	a.Append(3L);
	Anything_iterator ai = a.begin();
	for (long i = 0L; i < a.GetSize(); ++i, ++ai) {
		ASSERT(*ai == a[i]);
	}
	ASSERT(ai == a.end());
	const Anything &b = a;
	Anything_const_iterator bi = b.begin();
	for (long i = 0L; i < b.GetSize(); ++i, ++bi) {
		ASSERT(*bi == b[i]);
	}
	ASSERT(bi == b.end());
}
void AnythingIteratorTest::testSimpleAnythingIncDec() {
	Anything a(1L);
	Anything_iterator ai = a.begin();
	Anything_iterator ai1 = ++ai;
	ASSERT(ai == ai1);
	ASSERT(--ai == a.begin());
	ASSERT(ai1 == a.end());
	ASSERT(ai != ai1);
	ASSERT(--ai1 == ai);
	const Anything &b = a;
	Anything_const_iterator bi = b.begin();
	Anything_const_iterator bi1 = ++bi;
	ASSERT(bi == bi1);
	ASSERT(--bi == b.begin());
	ASSERT(bi1 == b.end());
	ASSERT(bi1 != bi);
	ASSERT(--bi == bi);
}
void AnythingIteratorTest::testAnythingIndex() {
	ASSERT_EQUAL(5, fAny5.GetSize());
	Anything_iterator it = fAny5.begin();
	ASSERT_EQUAL(fAny5[0], it[0]);
	ASSERT_EQUAL(fAny5[4], it[4]);
	++it;
	ASSERT_EQUAL(fAny5[4], it[3]);
	++it;
	ASSERT_EQUAL(fAny5[4], it[2]);
	ASSERT_EQUAL(fAny5.begin()[2], *it);
	ASSERT_EQUAL(*fAny5.begin(), it[-2]);
	ASSERT_EQUAL(fAny5.end()[-3], *it);
	Anything_const_iterator cit = fAny5.begin();
	ASSERT_EQUAL(fAny5[0], cit[0]);
	ASSERT_EQUAL(fAny5[4], cit[4]);
	++cit;
	ASSERT_EQUAL(fAny5[4], cit[3]);
	++cit;
	ASSERT_EQUAL(fAny5[4], cit[2]);
	ASSERT_EQUAL(fAny5.begin()[2], *cit);
	ASSERT_EQUAL(*fAny5.begin(), cit[-2]);
	ASSERT_EQUAL(fAny5.end()[-3], *cit);
}
void AnythingIteratorTest::testIteratorSubstract() {
	ASSERT_EQUAL(fAny5.GetSize(), fAny5.end() - fAny5.begin());
	const Anything &a5 = fAny5;
	ASSERT_EQUAL(a5.GetSize(), a5.end() - a5.begin());
	Anything a;
	const Anything &b(a);
	ASSERT_EQUAL(a.GetSize(), a.end() - a.begin());
	ASSERT_EQUAL(b.GetSize(), b.end() - b.begin());
	a.Append(1L);
	ASSERT_EQUAL(a.GetSize(), a.end() - a.begin());
	ASSERT_EQUAL(b.GetSize(), b.end() - b.begin());
	a.Append(2L);
	ASSERT_EQUAL(a.GetSize(), a.end() - a.begin());
	ASSERT_EQUAL(b.GetSize(), b.end() - b.begin());
	Anything_iterator ai = a.begin();
	++ai;
	ASSERT_EQUAL(1, ai - a.begin());
	ASSERT_EQUAL(-1, a.begin() - ai);
	Anything_const_iterator bi = b.begin();
	++bi;
	ASSERT_EQUAL(1, bi - b.begin());
	ASSERT_EQUAL(-1, b.begin() - bi);
}
void AnythingIteratorTest::testIteratorIntAdd() {
	Anything_iterator ai = fAny5.begin();
	ASSERT_EQUAL(*(ai + 4), fAny5[4]);
	++ai;
	ASSERT(ai == (fAny5.begin() + 1));
	ASSERT(ai == (fAny5.end() + (-4)));
	Anything_const_iterator bi = fAny5.begin();
	ASSERT_EQUAL(*(bi + 4), fAny5[4]);
	++bi;
	ASSERT(bi == (fAny5.begin() + 1));
	ASSERT(bi == (fAny5.end() + (-4)));
}
void AnythingIteratorTest::testIteratorIntSubstract() {
	Anything_iterator ai = fAny5.end();
	ASSERT_EQUAL(*(ai - 4), fAny5[1]);
	--ai;
	ASSERT(ai == (fAny5.end() - 1));
	ASSERT(ai == (fAny5.begin() - (-4)));
	Anything_const_iterator bi = fAny5.end();
	ASSERT_EQUAL(*(bi - 4), fAny5[1]);
	--bi;
	ASSERT(bi == (fAny5.end() - 1));
	ASSERT(bi == (fAny5.begin() - (-4)));
}
void AnythingIteratorTest::testIteratorCompare() {
	Anything a;
	ASSERT(a.begin() <= a.end());
	ASSERT(a.end() >= a.begin());
	ASSERT(a.begin() <= a.begin());
	ASSERT(a.begin() >= a.begin());
	ASSERT(!(a.begin() < a.begin()));
	ASSERT(!(a.begin() > a.begin()));
	const Anything &b(a);
	ASSERT(b.begin() <= b.end());
	ASSERT(b.end() >= b.begin());
	ASSERT(b.begin() <= b.begin());
	ASSERT(b.begin() >= b.begin());
	ASSERT(!(b.begin() < b.begin()));
	ASSERT(!(b.begin() > b.begin()));
	a.Append(1L);
	ASSERT(a.begin() < a.end());
	ASSERT(a.end() > a.begin());
	ASSERT(a.begin() <= a.end());
	ASSERT(a.end() >= a.begin());
	ASSERT(a.begin() <= a.begin());
	ASSERT(a.begin() >= a.begin());
	ASSERT(!(a.begin() < a.begin()));
	ASSERT(!(a.begin() > a.begin()));
	//- const_iterator
	ASSERT(b.begin() < b.end());
	ASSERT(b.end() > b.begin());
	ASSERT(b.begin() <= b.end());
	ASSERT(b.end() >= b.begin());
	ASSERT(b.begin() <= b.begin());
	ASSERT(b.begin() >= b.begin());
	ASSERT(!(b.begin() < b.begin()));
	ASSERT(!(b.begin() > b.begin()));

	ASSERT(fAny5.end() > fAny5.begin());
}
// the following 2 tests should belong to AnythingSTLTest,
// but for convenience of setUp with fAny5 it is here
void AnythingIteratorTest::testAnythingSingleErase() {
	Anything a = fAny5;
	ASSERT_EQUAL(5, a.end() - a.begin());
	ASSERT(a.begin() == a.erase(a.begin()));
	ASSERT_EQUAL(4, a.end() - a.begin());
	ASSERT(a.end() == a.erase(a.end()));// cannot delete past the end
	ASSERT_EQUAL(4, a.end() - a.begin());
	Anything_iterator aItEnd(a.end());
	ASSERT( aItEnd - 1 == a.erase( aItEnd - 1 ) );
	ASSERT_EQUAL(3, a.end() - a.begin());
	ASSERT(a.begin() + 1 == a.erase(a.begin() + 1));
	ASSERT_EQUAL(2, a.end() - a.begin());
	a.clear();
	ASSERT_EQUAL(0, a.end() - a.begin());
	ASSERT(a.empty());
}
void AnythingIteratorTest::testAnythingRangeErase() {
	Anything a = fAny5;
	ASSERT_EQUAL(5, a.end() - a.begin());
	ASSERT(a.end() == a.erase(a.end(), a.end()));
	ASSERT_EQUAL(5, a.end() - a.begin());
	ASSERT(a.begin() == a.erase(a.begin(), a.begin()));
	ASSERT_EQUAL(5, a.end() - a.begin());
	// until now, no-ops
	ASSERT(a.begin() == a.erase(a.begin(), a.begin() + 1));
	ASSERT_EQUAL(4, a.end() - a.begin());

}
AnythingIteratorTest::AnythingIteratorTest() {
	fAny5.clear();
	for (long i = 1; i <= 5; ++i) {
		fAny5.Append(i);
	}
}

void AnythingIteratorTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testEmptyAnythingBegin));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testSimpleAnythingBegin));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testSimpleAnythingDeref));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testSimpleAnythingIncrement));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testSimpleAnythingDecrement));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testSimpleAnythingIncDec));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testSimpleAnythingAssignment));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testAnythingIteration));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testAnythingIndex));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testIteratorSubstract));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testIteratorIntAdd));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testIteratorIntSubstract));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testIteratorCompare));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testAnythingSingleErase));
	s.push_back(CUTE_SMEMFUN(AnythingIteratorTest, testAnythingRangeErase));
}
