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

#include "StringSTLTest.h"

// the following 2 tests should belong to StringSTLTest,
// but for convenience of setUp with fStr5 it is here
void StringSTLTest::testStringSingleErase() {
	String a = fStr5;
	ASSERT_EQUAL(5, a.end() - a.begin());
	ASSERT(a.begin() == a.erase(a.begin()));
	ASSERT_EQUAL(4, a.end() - a.begin());
	ASSERT(a.end() == a.erase(a.end()));// cannot delete past the end
	ASSERT_EQUAL(4, a.end() - a.begin());
	String_iterator aItEnd(a.end());
	ASSERT( aItEnd - 1 == a.erase( aItEnd - 1 ) );
	ASSERT_EQUAL(3, a.end() - a.begin());
	ASSERT(a.begin() + 1 == a.erase(a.begin() + 1));
	ASSERT_EQUAL(2, a.end() - a.begin());
	a.clear();
	ASSERT_EQUAL(0, a.end() - a.begin());
	ASSERT(a.empty());
}
void StringSTLTest::testStringRangeErase() {
	{
		String a = fStr5;
		ASSERT_EQUAL(0L, a.erase().Length());
		ASSERT_EQUAL(String(), a);
	}
	{
		String a = fStr5;
		ASSERT_EQUAL(1L, a.erase(1).Length());
		ASSERT_EQUAL(fStr5.SubString(0L,1L), a);
	}
	{
		String a = fStr5;
		ASSERT_EQUAL(1L, a.erase(1, 20).Length());
		ASSERT_EQUAL(fStr5.SubString(0L,1L), a);
	}
	{
		String a = fStr5;
		ASSERT_EQUAL(2L, a.erase(1, 3).Length());
		ASSERT_EQUAL(String(fStr5.SubString(0,1)).Append(fStr5[4L]), a);
	}
}
void StringSTLTest::testStringRangeIteratorErase() {
	{
		String a = fStr5;
		ASSERT_EQUAL(5, a.end() - a.begin());
		ASSERT(a.end() == a.erase(a.end(), a.end()));
		ASSERT_EQUAL(5, a.end() - a.begin());
		ASSERT(a.begin() == a.erase(a.begin(), a.begin()));
		ASSERT_EQUAL(5, a.end() - a.begin());
		ASSERT(a.begin() == a.erase(a.begin(), a.begin() + 1));
		ASSERT_EQUAL(4, a.end() - a.begin());
		ASSERT_EQUAL(fStr5.SubString(1), a);
	}
	{
		String a = fStr5;
		ASSERT(a.begin()+1 == a.erase(a.begin()+1, a.begin() + 4));
		ASSERT_EQUAL(2, a.end() - a.begin());
		ASSERT_EQUAL(String(fStr5.SubString(0,1)).Append(fStr5[4L]), a);
	}
}
StringSTLTest::StringSTLTest() {
	fStr5.clear();
	for (long i = 1; i <= 5; ++i) {
		fStr5.Append(i);
	}
}

void StringSTLTest::testStringInsertIteratorSingle() {
	{
		String a = fStr5;
		long const l = a.Length();
		ASSERT(a.begin() == a.insert(a.begin(), 'z'));
		ASSERT_EQUAL(l + 1L, a.Length());
		ASSERT_EQUAL(String("z").Append(fStr5), a);
	}
	{
		String a = fStr5;
		long const l = a.Length();
		String::iterator end = a.end();
		ASSERT(end == a.insert(end, 'z'));
		ASSERT_EQUAL(l + 1L, a.Length());
		ASSERT_EQUAL(String(fStr5).Append('z'), a);
	}
	{
		String a = fStr5;
		long const l = a.Length();
		long const offset = 2L;
		String::iterator pos = a.begin()+offset;
		ASSERT(pos == a.insert(pos, 'z'));
		ASSERT_EQUAL(l + 1L, a.Length());
		ASSERT_EQUAL(String().Append(fStr5.SubString(0,offset)).Append('z').Append(fStr5.SubString(offset)), a);
	}
	{ //! illegal iterator passed
		String a = fStr5;
		long const l = a.Length();
		ASSERT(a.end() == a.insert(fStr5.end(), 'z'));
		ASSERT_EQUAL(l, a.Length());
		ASSERT_EQUAL(fStr5, a);
	}
}
void StringSTLTest::testStringInsertIteratorMultiple() {
	{
		String const chars("zzz");
		String a = fStr5;
		long const l = a.Length();
		long const sz = chars.Length();
		a.insert(a.begin(), sz, chars[0L]);
		ASSERT_EQUAL(l + sz, a.Length());
		ASSERT_EQUAL(String(chars).Append(fStr5), a);
	}
	{
		String const chars("zzz");
		String a = fStr5;
		long const l = a.Length();
		long const sz = chars.Length();
		a.insert(a.end(), sz, chars[0L]);
		ASSERT_EQUAL(l + sz, a.Length());
		ASSERT_EQUAL(String(fStr5).Append(chars), a);
	}
	{
		String const chars("zzz");
		String a = fStr5;
		long const l = a.Length();
		long const offset = 2L;
		long const sz = chars.Length();
		String::iterator pos = a.begin()+offset;
		a.insert(pos, sz, chars[0L]);
		ASSERT_EQUAL(l + sz, a.Length());
		ASSERT_EQUAL(String().Append(fStr5.SubString(0,offset)).Append(chars).Append(fStr5.SubString(offset)), a);
	}
}
void StringSTLTest::testStringInsertIteratorRange() {
	{
		String const chars("xyz");
		String a = fStr5;
		long const l = a.Length();
		long const sz = chars.Length();
		a.insert(a.begin(), chars.begin(), chars.end());
		ASSERT_EQUAL(l + sz, a.Length());
		ASSERT_EQUAL(String(chars).Append(fStr5), a);
	}
	{
		String const chars("xyz");
		String a = fStr5;
		long const l = a.Length();
		long const sz = chars.Length();
		a.insert(a.end(), chars.begin(), chars.end());
		ASSERT_EQUAL(l + sz, a.Length());
		ASSERT_EQUAL(String(fStr5).Append(chars), a);
	}
	{
		String const chars("xyz");
		String a = fStr5;
		long const l = a.Length();
		long const offset = 2L;
		long const sz = chars.Length();
		String::iterator pos = a.begin()+offset;
		a.insert(pos, chars.begin(), chars.end());
		ASSERT_EQUAL(l + sz, a.Length());
		ASSERT_EQUAL(String().Append(fStr5.SubString(0,offset)).Append(chars).Append(fStr5.SubString(offset)), a);
	}
}

void StringSTLTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(StringSTLTest, testStringSingleErase));
	s.push_back(CUTE_SMEMFUN(StringSTLTest, testStringRangeErase));
	s.push_back(CUTE_SMEMFUN(StringSTLTest, testStringRangeIteratorErase));
	s.push_back(CUTE_SMEMFUN(StringSTLTest, testStringInsertIteratorSingle));
	s.push_back(CUTE_SMEMFUN(StringSTLTest, testStringInsertIteratorMultiple));
	s.push_back(CUTE_SMEMFUN(StringSTLTest, testStringInsertIteratorRange));
}
