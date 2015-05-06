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

#include "StringReverseIteratorTest.h"

void StringReverseIteratorTest::testEmptyStringBegin() {
	String a;
	ASSERT(a.rbegin() == a.rend());
	const String &b = a;
	ASSERT(b.rbegin() == b.rend());
}
void StringReverseIteratorTest::testSimpleStringBegin() {
	String a("bla");
	ASSERT(a.rbegin() != a.rend());
	const String &b = a;
	ASSERT(b.rbegin() != b.rend());
}
void StringReverseIteratorTest::testSimpleStringDeref() {
	String a("bla");
	ASSERT_EQUAL(*a.rbegin(), a[2L]);
	const String &b = a;
	ASSERT_EQUAL(*b.rbegin(), b[2L]);
}
void StringReverseIteratorTest::testSimpleStringIncrement() {
	String a("b");
	String::reverse_iterator ai = a.rbegin();
	ASSERT(++ai == a.rend());
	ASSERT(ai != a.rbegin());
	const String &b = a;
	String::const_reverse_iterator bi = b.rbegin();
	ASSERT(++bi == b.rend());
	ASSERT(bi != b.rbegin());
}
void StringReverseIteratorTest::testSimpleStringDecrement() {
	String a("b");
	String::reverse_iterator ai = a.rend();
	ASSERT(--ai == a.rbegin());
	ASSERT(ai != a.rend());
	const String &b = a;
	String::const_reverse_iterator bi = b.rend();
	ASSERT(--bi == b.rbegin());
	ASSERT(bi != b.rend());
}
void StringReverseIteratorTest::testSimpleStringAssignment() {
	String a("bla");
	*a.rbegin() = 'x';
	String::const_reverse_iterator ai = a.rbegin();
	ASSERT(*ai == 'x');
	ASSERT(*++ai == 'l');
	ASSERT(*++ai == 'b');
	ASSERT(*++ai == '\0');
}
void StringReverseIteratorTest::testStringIteration() {
	String a;
	a.Append(1L);
	a.Append(2L);
	a.Append(3L);
	String::reverse_iterator ai = a.rbegin();
	for (long i = a.size() - 1; i >= 0; --i, ++ai) {
		ASSERT(*ai == a[i]);
	}
	ASSERT(ai == a.rend());
	const String &b = a;
	String::const_reverse_iterator bi = b.rbegin();
	for (long i = b.size() - 1; i >= 0; --i, ++bi) {
		ASSERT(*bi == b[i]);
	}
	ASSERT(bi == b.rend());
}
void StringReverseIteratorTest::testSimpleStringIncDec() {
	String a("b");
	String::reverse_iterator ai = a.rbegin();
	String::reverse_iterator ai1 = ++ai;
	ASSERT(ai == ai1);
	ASSERT(--ai == a.rbegin());
	ASSERT(ai1 == a.rend());
	ASSERT(ai != ai1);
	ASSERT(--ai1 == ai);
	const String &b = a;
	String::const_reverse_iterator bi = b.rbegin();
	String::const_reverse_iterator bi1 = ++bi;
	ASSERT(bi == bi1);
	ASSERT(--bi == b.rbegin());
	ASSERT(bi1 == b.rend());
	ASSERT(bi1 != bi);
	ASSERT(--bi == bi);
}
void StringReverseIteratorTest::testStringIndex() {
	ASSERT_EQUAL(5, fStr5.size());
	String::reverse_iterator it = fStr5.rbegin();
	ASSERT_EQUAL(fStr5[4], it[0]);
	ASSERT_EQUAL(fStr5[0], it[4]);
	++it;
	ASSERT_EQUAL(fStr5[0], it[3]);
	++it;
	ASSERT_EQUAL(fStr5[0], it[2]);
	ASSERT_EQUAL(fStr5.rbegin()[2], *it);
	ASSERT_EQUAL(*fStr5.rbegin(), it[-2]);
	ASSERT_EQUAL(fStr5.rend()[-3], *it);
	String::const_reverse_iterator cit = fStr5.rbegin();
	ASSERT_EQUAL(fStr5[4], cit[0]);
	ASSERT_EQUAL(fStr5[0], cit[4]);
	++cit;
	ASSERT_EQUAL(fStr5[0], cit[3]);
	++cit;
	ASSERT_EQUAL(fStr5[0], cit[2]);
	ASSERT_EQUAL(fStr5.rbegin()[2], *cit);
	ASSERT_EQUAL(*fStr5.rbegin(), cit[-2]);
	ASSERT_EQUAL(fStr5.rend()[-3], *cit);
}
void StringReverseIteratorTest::testIteratorSubstract() {
	ASSERT_EQUAL(fStr5.size(), fStr5.rend() - fStr5.rbegin());
	const String &a5 = fStr5;
	ASSERT_EQUAL(a5.size(), a5.rend() - a5.rbegin());
	String a;
	const String &b(a);
	ASSERT_EQUAL(a.size(), a.rend() - a.rbegin());
	ASSERT_EQUAL(b.size(), b.rend() - b.rbegin());
	a.Append(1L);
	ASSERT_EQUAL(a.size(), a.rend() - a.rbegin());
	ASSERT_EQUAL(b.size(), b.rend() - b.rbegin());
	a.Append(2L);
	ASSERT_EQUAL(a.size(), a.rend() - a.rbegin());
	ASSERT_EQUAL(b.size(), b.rend() - b.rbegin());
	String::reverse_iterator ai = a.rbegin();
	++ai;
	ASSERT_EQUAL(1, ai - a.rbegin());
	ASSERT_EQUAL(-1, a.rbegin() - ai);
	String::const_reverse_iterator bi = b.rbegin();
	++bi;
	ASSERT_EQUAL(1, bi - b.rbegin());
	ASSERT_EQUAL(-1, b.rbegin() - bi);
}
void StringReverseIteratorTest::testIteratorIntAdd() {
	String::reverse_iterator ai = fStr5.rbegin();
	ASSERT_EQUAL(fStr5[0], *(ai + 4));
	++ai;
	ASSERT(ai == (fStr5.rbegin() + 1));
	ASSERT(ai == (fStr5.rend() + (-4)));
	String::const_reverse_iterator bi = fStr5.rbegin();
	ASSERT_EQUAL(fStr5[0], *(bi + 4));
	++bi;
	ASSERT(bi == (fStr5.rbegin() + 1));
	ASSERT(bi == (fStr5.rend() + (-4)));
}
void StringReverseIteratorTest::testIteratorIntSubstract() {
	String::reverse_iterator ai = fStr5.rend();
	ASSERT_EQUAL(fStr5[3], *(ai - 4));
	--ai;
	ASSERT(ai == (fStr5.rend() - 1));
	ASSERT(ai == (fStr5.rbegin() - (-4)));
	String::const_reverse_iterator bi = fStr5.rend();
	ASSERT_EQUAL(fStr5[3], *(bi - 4));
	--bi;
	ASSERT(bi == (fStr5.rend() - 1));
	ASSERT(bi == (fStr5.rbegin() - (-4)));
}
void StringReverseIteratorTest::testIteratorCompare() {
	String a;
	ASSERT(a.rbegin() <= a.rend());
	ASSERT(a.rend() >= a.rbegin());
	ASSERT(a.rbegin() <= a.rbegin());
	ASSERT(a.rbegin() >= a.rbegin());
	ASSERT(!(a.rbegin() < a.rbegin()));
	ASSERT(!(a.rbegin() > a.rbegin()));
	const String &b(a);
	ASSERT(b.rbegin() <= b.rend());
	ASSERT(b.rend() >= b.rbegin());
	ASSERT(b.rbegin() <= b.rbegin());
	ASSERT(b.rbegin() >= b.rbegin());
	ASSERT(!(b.rbegin() < b.rbegin()));
	ASSERT(!(b.rbegin() > b.rbegin()));
	a.Append(1L);
	ASSERT(a.rbegin() < a.rend());
	ASSERT(a.rend() > a.rbegin());
	ASSERT(a.rbegin() <= a.rend());
	ASSERT(a.rend() >= a.rbegin());
	ASSERT(a.rbegin() <= a.rbegin());
	ASSERT(a.rbegin() >= a.rbegin());
	ASSERT(!(a.rbegin() < a.rbegin()));
	ASSERT(!(a.rbegin() > a.rbegin()));
	//- const_iterator
	ASSERT(b.rbegin() < b.rend());
	ASSERT(b.rend() > b.rbegin());
	ASSERT(b.rbegin() <= b.rend());
	ASSERT(b.rend() >= b.rbegin());
	ASSERT(b.rbegin() <= b.rbegin());
	ASSERT(b.rbegin() >= b.rbegin());
	ASSERT(!(b.rbegin() < b.rbegin()));
	ASSERT(!(b.rbegin() > b.rbegin()));

	ASSERT(fStr5.rend() > fStr5.rbegin());
}
StringReverseIteratorTest::StringReverseIteratorTest() {
	fStr5.clear();
	for (long i = 1; i <= 5; ++i) {
		fStr5.Append(i);
	}
}
void StringReverseIteratorTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testEmptyStringBegin));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testSimpleStringBegin));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testSimpleStringDeref));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testSimpleStringIncrement));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testSimpleStringDecrement));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testSimpleStringIncDec));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testSimpleStringAssignment));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testStringIteration));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testStringIndex));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testIteratorSubstract));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testIteratorIntAdd));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testIteratorIntSubstract));
	s.push_back(CUTE_SMEMFUN(StringReverseIteratorTest, testIteratorCompare));
}
