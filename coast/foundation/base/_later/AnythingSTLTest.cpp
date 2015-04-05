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

#include "AnythingSTLTest.h"

void AnythingSTLTest::testSimpleSwap() {
	Anything first(1L);
	Anything second("two");
	first.swap(second);
	ASSERT_EQUAL(1L, second.AsLong());
	ASSERT_EQUAL("two", first.AsCharPtr());
}
void AnythingSTLTest::testSwapWithDifferentAllocator() {
	GlobalAllocator ga;
	Anything first(1L, &ga);
	ASSERT(&ga == first.GetAllocator());
	Anything second("two"); // normal allocator
	ASSERT(coast::storage::Current() == second.GetAllocator());
	ASSERT(&ga != coast::storage::Current());
	first.swap(second);
	ASSERT_EQUAL(1L, second.AsLong());
	ASSERT_EQUAL("two", first.AsCharPtr());
	ASSERT(&ga == second.GetAllocator());
	ASSERT(coast::storage::Current() == first.GetAllocator());
}
void AnythingSTLTest::testFrontBackPushPop() {
	Anything a;
	a.push_back("two");
	a.push_front(1L);
	a.push_back(3.14);
	ASSERT_EQUAL(3, a.size());
	ASSERT_EQUAL(1L, a.front().AsLong());
	ASSERT_EQUAL_DELTA(3.14, a.back().AsDouble(), 0.0000000001);
	a.pop_back();
	ASSERT_EQUAL("two", a.back().AsCharPtr());
	ASSERT_EQUAL(1L, a.front().AsLong());
	a.pop_front();
	ASSERT_EQUAL("two", a.front().AsCharPtr());
	ASSERT_EQUAL("two", a.back().AsCharPtr());
	a.pop_front();
	ASSERT(a.empty());
	a.pop_back();
	ASSERT(a.empty());
}
static const long rangeinput[] = { 1, 2, 3, 4, 5 };
const int rangeSize = (sizeof(rangeinput) / sizeof(rangeinput[0]));

void AnythingSTLTest::checkRange(const Anything &a, long n = 0, long lengthOfCopy = rangeSize) {
	for (int i = 0; i < lengthOfCopy; ++i) {
		ASSERT_EQUAL(rangeinput[i], a[i+n].AsLong());
	}
}

void AnythingSTLTest::testRangeAssign() {
	Anything a;
	a.assign(rangeinput, rangeinput + rangeSize);
	checkRange(a);
}
void AnythingSTLTest::testRangeCtor() {
	Anything a(rangeinput, rangeinput + rangeSize);
	checkRange(a);
	Anything b(rangeinput, rangeinput + rangeSize, coast::storage::Current());
	checkRange(b);
}
const Anything::size_type nOfCopies = 4;
const char *valueToBeCopied = "a test";
void AnythingSTLTest::checkFill(const Anything &a) {
	ASSERT_EQUAL(nOfCopies, a.size());
	for (int i = 0; i < nOfCopies; ++i) {
		ASSERT_EQUAL(valueToBeCopied, a[i].AsCharPtr());
	}
}
void AnythingSTLTest::checkFillSizeType(const Anything &a) {
	for (long i = 0; i < nOfCopies; ++i) {
		ASSERT_EQUAL(42L, a[i].AsLong());
	}
}
void AnythingSTLTest::testFillAssign() {
	Anything a;
	a.assign(nOfCopies, valueToBeCopied);
	checkFill(a);
}
void AnythingSTLTest::testFillAssignWithSizeType() {
	Anything a;
	a.assign(nOfCopies, Anything::size_type(42L));
	checkFillSizeType(a);
}
void AnythingSTLTest::testFillCtor() {
	Anything a(nOfCopies, valueToBeCopied);
	checkFill(a);
}
void AnythingSTLTest::testFillCtorWithSizeType() {
	Anything a(nOfCopies, Anything::size_type(42L));
	checkFillSizeType(a);
	Anything b(nOfCopies, Anything::size_type(42L), coast::storage::Current());
	checkFillSizeType(b);
}
void AnythingSTLTest::testSimpleInsertToEmptyAny() {
	Anything a;
	a.insert(a.begin(), 42L);
	ASSERT_EQUAL(42L, a[0L].AsLong());
	a.clear();
	ASSERT(a.IsNull());
	a.insert(a.end(), "test");
	ASSERT_EQUAL("test", a[0L].AsCharPtr());
}
void AnythingSTLTest::checkSimpleInsert(const Anything &a, const char *m) {
	ASSERT_EQUALM( m,42L, a[0].AsLong());
	ASSERT_EQUALM( m,"test", a[1].AsCharPtr());
	ASSERT_EQUALM( m,2, a.size());
}

void AnythingSTLTest::testSimpleInsertAtEnd() {
	Anything a(42L);
	a.insert(a.end(), "test");
	checkSimpleInsert(a, "testSimpleInsertAtEnd");
}
void AnythingSTLTest::testSimpleInsertAtBegin() {
	Anything a("test");
	a.insert(a.begin(), 42L);
	checkSimpleInsert(a, "testSimpleInsertAtBegin");
}
void AnythingSTLTest::checkInsertInArray(const Anything &a, long testpos, const char *msg, long n) {
	String m(msg);
	m.Append(" pos: ");
	m.Append(testpos);
	ASSERT_EQUALM( m,rangeSize + n, a.size());
	for (long i = 0L; i < n; ++i) {
		ASSERT_EQUALM( m,42L, a[testpos+i].AsLong());
	}
	for (long i = 0L; i < testpos; ++i) {
		ASSERT_EQUALM( m,rangeinput[i], a[i].AsLong());
	}
	for (long i = testpos; i < rangeSize; ++i) {
		ASSERT_EQUALM( m,rangeinput[i], a[i+n].AsLong());
	}
}
void AnythingSTLTest::testSimpleInsertInArray() {
	for (long testpos = 0; testpos <= rangeSize; ++testpos) {
		Anything a(rangeinput, rangeinput + rangeSize);
		a.insert(a.begin() + testpos, 42L);
		checkInsertInArray(a, testpos, "SimpleInsert");
	}
}
Anything makeAnyWithKeys() {
	Anything a;
	a["eins"] = 1L;
	a["zwei"] = 2.0;
	a.Append("drei");
	a["vier"] = "vier";
	return a;
}
void AnythingSTLTest::checkInsertWithKeys(const Anything &a, const long testpos, const char *m, const long n) {
	String msg(m);
	msg.Append(":run ");
	msg.Append(testpos);
	long einspos = (testpos > 0 ? 0L : n);
	ASSERT_EQUALM( msg,"eins", a.SlotName(einspos));
	ASSERT_EQUALM( msg,1L, a["eins"].AsLong());
	ASSERT_EQUAL_DELTAm(2.0, a["zwei"].AsDouble(), 0.000001, msg);
	long dreipos = a.size() - (testpos < 3 ? 2L : 2 + n);
	ASSERT_EQUALM( msg,"drei", a[dreipos].AsCharPtr());
	long vierpos = a.size() - (testpos < 4 ? 1L : 1L + n);
	ASSERT_EQUALM( msg,"vier", a[vierpos].AsCharPtr());
	ASSERT_EQUALM( msg,"vier", a.SlotName(vierpos));
}
void AnythingSTLTest::testSimpleInsertWithKeys() {
	for (long testpos = 0; testpos <= 4; ++testpos) {
		Anything a(makeAnyWithKeys());
		a.insert(a.begin() + testpos, 42L);
		checkInsertWithKeys(a, testpos, "SimpleInsert");
		ASSERT_EQUAL(42L, a[testpos].AsLong());
	}
}
void AnythingSTLTest::testFillInsertToEmptyAny() {
	Anything a;
	const Anything::size_type n = 4;
	const char *content = "foo";
	a.insert(a.begin(), n, content);
	ASSERT_EQUAL(n, a.size());
	for (long i = 0; i < n; ++i) {
		ASSERT_EQUAL(content, a[i].AsCharPtr());
	}
}
void AnythingSTLTest::testFillInsertInArray() {
	for (long testpos = 0; testpos <= rangeSize; ++testpos) {
		Anything a(rangeinput, rangeinput + rangeSize);
		const long n = 3;
		a.insert(a.begin() + testpos, n, 42L);
		checkInsertInArray(a, testpos, "FillInsert", n);
	}
}
void AnythingSTLTest::testFillInsertWithKeys() {
	for (long testpos = 0; testpos <= 4; ++testpos) {
		const long n = 5;
		Anything a(makeAnyWithKeys());
		a.insert(a.begin() + testpos, n, 42L);
		checkInsertWithKeys(a, testpos, "FillInsert", n);
		for (long i = testpos; i < testpos + n; ++i) {
			ASSERT_EQUAL(42L, a[i].AsLong());
		}
	}
}
void AnythingSTLTest::testFillInsertZero() {
	Anything a;
	a.insert(a.begin(), 0, 42L);
	ASSERT_EQUAL(Anything(), a);
	a.insert(a.begin(), 1L);
	ASSERT_EQUAL(1, a.size());
	a.insert(a.begin(), 0, 42L); // no op!
	ASSERT_EQUAL(1L, a.AsLong());
	a["two"] = "zwei";
	a.insert(a.begin() + 1, 0, 42L); // no op
	ASSERT_EQUAL(2, a.size());
	ASSERT_EQUAL("zwei", a[1L].AsCharPtr());
}

void AnythingSTLTest::testIterInsertToEmptyAny() {
	Anything a;
	a.insert(a.begin(), rangeinput, rangeinput + rangeSize);
	checkRange(a);
}
void AnythingSTLTest::testIterInsertInArray() {
	for (long testpos = 0; testpos <= rangeSize; ++testpos) {
		Anything a(rangeinput, rangeinput + rangeSize);
		const long lengthOfInsertion = rangeSize - 1;
		a.insert(a.begin() + testpos, rangeinput, rangeinput + lengthOfInsertion);
		String m(" pos: ");
		m.Append(testpos);
		ASSERT_EQUALM( m,rangeSize + lengthOfInsertion, a.size());
		checkRange(a, testpos, lengthOfInsertion);
		for (long i = 0L; i < lengthOfInsertion; ++i) {
			ASSERT_EQUALM( m,rangeinput[i], a[testpos+i].AsLong());
		}
		for (long i = 0L; i < testpos; ++i) {
			ASSERT_EQUALM( m,rangeinput[i], a[i].AsLong());
		}
		for (long i = testpos; i < rangeSize; ++i) {
			ASSERT_EQUALM( m,rangeinput[i], a[i+lengthOfInsertion].AsLong());
		}
	}
}
void AnythingSTLTest::testIterInsertWithKeys() {
	for (long testpos = 0; testpos <= 4; ++testpos) {
		const long lengthOfInsertion = rangeSize - 1;
		Anything a(makeAnyWithKeys());
		a.insert(a.begin() + testpos, rangeinput, rangeinput + lengthOfInsertion);
		checkInsertWithKeys(a, testpos, "FillInsert", lengthOfInsertion);
		checkRange(a, testpos, lengthOfInsertion);
	}
}
void AnythingSTLTest::testIterInsertZero() {
	Anything a;
	a.insert(a.begin(), rangeinput, rangeinput); // no op!
	ASSERT_EQUAL(Anything(), a);
	a.insert(a.begin(), rangeinput, rangeinput + 1);
	ASSERT_EQUAL(1, a.size());
	a.insert(a.begin(), rangeinput, rangeinput); // no op!
	ASSERT_EQUAL(1L, a.AsLong());
	a["two"] = "zwei";
	a.insert(a.begin() + 1, rangeinput, rangeinput); // no op
	ASSERT_EQUAL(2, a.size());
	ASSERT_EQUAL("zwei", a[1L].AsCharPtr());
}
void AnythingSTLTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testSimpleSwap));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testSwapWithDifferentAllocator));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testFrontBackPushPop));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testRangeAssign));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testFillAssign));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testFillAssignWithSizeType));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testRangeCtor));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testFillCtor));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testFillCtorWithSizeType));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testSimpleInsertToEmptyAny));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testSimpleInsertAtEnd));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testSimpleInsertAtBegin));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testSimpleInsertInArray));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testSimpleInsertWithKeys));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testFillInsertToEmptyAny));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testFillInsertInArray));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testFillInsertWithKeys));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testFillInsertZero));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testIterInsertToEmptyAny));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testIterInsertInArray));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testIterInsertWithKeys));
	s.push_back(CUTE_SMEMFUN(AnythingSTLTest, testIterInsertZero));
}
