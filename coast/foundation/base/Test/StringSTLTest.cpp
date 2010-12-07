/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringSTLTest.h"
#include "TestSuite.h"

// the following 2 tests should belong to StringSTLTest,
// but for convenience of setUp with fStr5 it is here
void StringSTLTest::testStringSingleErase() {
	String a = fStr5;
	assertEqual(5, a.end() - a.begin());
	t_assert(a.begin() == a.erase(a.begin()));
	assertEqual(4, a.end() - a.begin());
	t_assert(a.end() == a.erase(a.end()));// cannot delete past the end
	assertEqual(4, a.end() - a.begin());
	String_iterator aItEnd(a.end());
	t_assert( aItEnd - 1 == a.erase( aItEnd - 1 ) );
	assertEqual(3, a.end() - a.begin());
	t_assert(a.begin() + 1 == a.erase(a.begin() + 1));
	assertEqual(2, a.end() - a.begin());
	a.clear();
	assertEqual(0, a.end() - a.begin());
	t_assert(a.empty());
}
void StringSTLTest::testStringRangeErase() {
	{
		String a = fStr5;
		assertEqual(0L, a.erase().Length());
		assertCharPtrEqual(String(), a);
	}
	{
		String a = fStr5;
		assertEqual(1L, a.erase(1).Length());
		assertCharPtrEqual(fStr5.SubString(0L,1L), a);
	}
	{
		String a = fStr5;
		assertEqual(1L, a.erase(1, 20).Length());
		assertCharPtrEqual(fStr5.SubString(0L,1L), a);
	}
	{
		String a = fStr5;
		assertEqual(2L, a.erase(1, 3).Length());
		assertCharPtrEqual(String(fStr5.SubString(0,1)).Append(fStr5[4L]), a);
	}
}
void StringSTLTest::testStringRangeIteratorErase() {
	{
		String a = fStr5;
		assertEqual(5, a.end() - a.begin());
		t_assert(a.end() == a.erase(a.end(), a.end()));
		assertEqual(5, a.end() - a.begin());
		t_assert(a.begin() == a.erase(a.begin(), a.begin()));
		assertEqual(5, a.end() - a.begin());
		t_assert(a.begin() == a.erase(a.begin(), a.begin() + 1));
		assertEqual(4, a.end() - a.begin());
		assertCharPtrEqual(fStr5.SubString(1), a);
	}
	{
		String a = fStr5;
		t_assert(a.begin()+1 == a.erase(a.begin()+1, a.begin() + 4));
		assertEqual(2, a.end() - a.begin());
		assertCharPtrEqual(String(fStr5.SubString(0,1)).Append(fStr5[4L]), a);
	}
}
void StringSTLTest::setUp() {
	fStr5.clear();
	for (long i = 1; i <= 5; ++i) {
		fStr5.Append(i);
	}
}

void StringSTLTest::testStringInsertIteratorSingle() {
	{
		String a = fStr5;
		long const l = a.Length();
		t_assert(a.begin() == a.insert(a.begin(), 'z'));
		assertEqual(l + 1L, a.Length());
		assertCharPtrEqual(String("z").Append(fStr5), a);
	}
	{
		String a = fStr5;
		long const l = a.Length();
		String::iterator end = a.end();
		t_assert(end == a.insert(end, 'z'));
		assertEqual(l + 1L, a.Length());
		assertCharPtrEqual(String(fStr5).Append('z'), a);
	}
	{
		String a = fStr5;
		long const l = a.Length();
		long const offset = 2L;
		String::iterator pos = a.begin()+offset;
		t_assert(pos == a.insert(pos, 'z'));
		assertEqual(l + 1L, a.Length());
		assertCharPtrEqual(String().Append(fStr5.SubString(0,offset)).Append('z').Append(fStr5.SubString(offset)), a);
	}
	{ //! illegal iterator passed
		String a = fStr5;
		long const l = a.Length();
		t_assert(a.end() == a.insert(fStr5.end(), 'z'));
		assertEqual(l, a.Length());
		assertCharPtrEqual(fStr5, a);
	}
}
void StringSTLTest::testStringInsertIteratorMultiple() {
	{
		String const chars("zzz");
		String a = fStr5;
		long const l = a.Length();
		long const sz = chars.Length();
		a.insert(a.begin(), sz, chars[0L]);
		assertEqual(l + sz, a.Length());
		assertCharPtrEqual(String(chars).Append(fStr5), a);
	}
	{
		String const chars("zzz");
		String a = fStr5;
		long const l = a.Length();
		long const sz = chars.Length();
		a.insert(a.end(), sz, chars[0L]);
		assertEqual(l + sz, a.Length());
		assertCharPtrEqual(String(fStr5).Append(chars), a);
	}
	{
		String const chars("zzz");
		String a = fStr5;
		long const l = a.Length();
		long const offset = 2L;
		long const sz = chars.Length();
		String::iterator pos = a.begin()+offset;
		a.insert(pos, sz, chars[0L]);
		assertEqual(l + sz, a.Length());
		assertCharPtrEqual(String().Append(fStr5.SubString(0,offset)).Append(chars).Append(fStr5.SubString(offset)), a);
	}
}
void StringSTLTest::testStringInsertIteratorRange() {
	{
		String const chars("xyz");
		String a = fStr5;
		long const l = a.Length();
		long const sz = chars.Length();
		a.insert(a.begin(), chars.begin(), chars.end());
		assertEqual(l + sz, a.Length());
		assertCharPtrEqual(String(chars).Append(fStr5), a);
	}
	{
		String const chars("xyz");
		String a = fStr5;
		long const l = a.Length();
		long const sz = chars.Length();
		a.insert(a.end(), chars.begin(), chars.end());
		assertEqual(l + sz, a.Length());
		assertCharPtrEqual(String(fStr5).Append(chars), a);
	}
	{
		String const chars("xyz");
		String a = fStr5;
		long const l = a.Length();
		long const offset = 2L;
		long const sz = chars.Length();
		String::iterator pos = a.begin()+offset;
		a.insert(pos, chars.begin(), chars.end());
		assertEqual(l + sz, a.Length());
		assertCharPtrEqual(String().Append(fStr5.SubString(0,offset)).Append(chars).Append(fStr5.SubString(offset)), a);
	}
}

Test *StringSTLTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, StringSTLTest, testStringSingleErase);
	ADD_CASE(testSuite, StringSTLTest, testStringRangeErase);
	ADD_CASE(testSuite, StringSTLTest, testStringRangeIteratorErase);
	ADD_CASE(testSuite, StringSTLTest, testStringInsertIteratorSingle);
	ADD_CASE(testSuite, StringSTLTest, testStringInsertIteratorMultiple);
	ADD_CASE(testSuite, StringSTLTest, testStringInsertIteratorRange);
	return testSuite;
}
