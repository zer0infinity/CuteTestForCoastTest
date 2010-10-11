/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnythingKeyIndexTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "IFAObject.h"
#include "Dbg.h"
#include "AnyIterators.h"
#include "SystemLog.h"

//--- c-library modules used ---------------------------------------------------
#include <cstring>

//---- AnythingKeyIndexTest ---------------------------------------------------------

AnythingKeyIndexTest::AnythingKeyIndexTest(TString tname) :
	TestCaseType(tname)
{
}

void AnythingKeyIndexTest::setUp()
{
	StartTrace(AnythingKeyIndexTest.setUp);
	fArray["slot"] = "contents";
	fArray["second"] = "more contents";
	fSequence.Append("A String");
	fSequence.Append("Another String");
}

class DummyIFAObj: public IFAObject
{
public:
	DummyIFAObj(const char *) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) DummyIFAObj("dummy");
	}
};

Test *AnythingKeyIndexTest::suite()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnythingKeyIndexTest, SimpleRemove);
	ADD_CASE(testSuite, AnythingKeyIndexTest, RemoveInvKeys);
	ADD_CASE(testSuite, AnythingKeyIndexTest, IndexAccess);
	ADD_CASE(testSuite, AnythingKeyIndexTest, KeyAccess0);
	ADD_CASE(testSuite, AnythingKeyIndexTest, KeyAccess1);
	ADD_CASE(testSuite, AnythingKeyIndexTest, KeyAccess2);
	ADD_CASE(testSuite, AnythingKeyIndexTest, MixKeysAndArray);
	ADD_CASE(testSuite, AnythingKeyIndexTest, Recursive);
	ADD_CASE(testSuite, AnythingKeyIndexTest, EmptyAccess0);
	ADD_CASE(testSuite, AnythingKeyIndexTest, EmptyAccess1);
	return testSuite;
}

Anything AnythingKeyIndexTest::init5DimArray(long anzElt)
{
	long i0, i1;
	char idx0[3] = { 0 }, idx1[3] = { 0 };
	Anything anyInit;

	for (i0 = '0'; i0 < anzElt + '0'; i0++) {
		long l0 = (long) (i0 - '0');
		idx0[0L] = (char) i0;

		for (i1 = '0'; i1 < anzElt + '0'; i1++) {
			long l1 = (long) (i1 - '0');
			idx1[0L] = (char) i1;
			anyInit[idx0][idx1] = (l0) + (l1);
		}
	}
	return (anyInit);
}

bool AnythingKeyIndexTest::check5DimArray(Anything &any0, Anything &any1, long anzElt)
{
	long i0, i1;
	char idx0[3] = { 0 }, idx1[3] = { 0 };
	bool retVal = true;
	Anything any;

	for (i0 = '0'; i0 < anzElt + '0'; i0++) {
		idx0[0L] = (char) i0;
		if (any0[idx0].At("0").At("0").At("0").At("0")
			!= any1[idx0].At("0").At("0").At("0").At("0")) {
			retVal = false;
		}
		for (i1 = '0'; i1 < anzElt + '0'; i1++) {
			idx1[0L] = (char) i1;
			if (any0[idx0][idx1].At("0").At("0").At("0")
				!= any1[idx0][idx1].At("0").At("0").At("0")) {
				retVal = false;
			}
		}
	}
	return (retVal);
}

void AnythingKeyIndexTest::RemoveInvKeys()
{
	long i0, i1, i2;
	Anything any0;

	// Prepare:  Fill an anything
	any0 = AnythingKeyIndexTest::init5DimArray(2);

	long sizeBefore = any0.GetSize();
	char buf[4] = { 0 };

	// Proper deletion test:  remove invalid SlotNames
	for (i0 = 'a'; i0 < 'z'; i0++) {
		buf[0L] = (char) i0;
		for (i1 = 'A'; i1 < 'Z'; i1++) {
			buf[1L] = (char) i1;
			for (i2 = '0'; i2 < '9'; i2++) {
				buf[2L] = (char) i2;
				any0.Remove(buf);
				assertCompare( sizeBefore, equal_to, any0.GetSize() );
				t_assert( any0.IsDefined( buf ) == false );
			}
		}
	}

	// Remove indexes which do not exist
	for (i0 = 1000000; i0 < 1002000; i0 += 137) {
		any0.Remove(i0);
		assertCompare( sizeBefore, equal_to, any0.GetSize() );
		//		t_assert( any0.IsDefined( i0 ) == false );
	}

	// Remove negative indexes
	for (i0 = -1000L; i0 < 0L; i0 += 13) {
		any0.Remove(i0); // alert, but no crash
		assertCompare( sizeBefore, equal_to, any0.GetSize() );
		//		t_assert( any0.IsDefined( i0 ) == false );
	}

	// Check if any0 till unchanged
	t_assert( AnythingKeyIndexTest::check5DimArray(any0, any0, 2) == true );
}

void AnythingKeyIndexTest::SimpleRemove()
{
	Anything simple;
	bool res;
	String resString;

	simple["one"] = "One";
	simple["two"] = "Two";
	simple["three"] = "Three";

	simple.Remove("two");
	res = simple.IsDefined("two");

	assertEqual(false, res);

	resString = simple.SlotName(0);
	assertEqual("one", resString);

	resString = simple.SlotName(1);
	assertEqual("three", resString);

	resString = simple.SlotName(2);
	assertEqual("", resString);
}

void AnythingKeyIndexTest::IndexAccess()
/* what: test access to Anythings through legal and illegal numeric indices*/
{
	long lengthBefore = fSequence.GetSize();

	t_assert(fSequence[0L].AsString("") == "A String");
	t_assert(fSequence[1].AsString("") == "Another String");
	assertCompare( fSequence.GetSize(), equal_to, lengthBefore );
	int newMax = lengthBefore + 2;
	t_assert(fSequence[newMax].AsString("") == "");
	t_assert(fSequence.GetSize() == newMax + 1);
}

void AnythingKeyIndexTest::KeyAccess0()
/* what: test access to Anythings through regular string keys*/
{
	long lengthBefore = fArray.GetSize();

	t_assert(fArray["slot"] == "contents");
	t_assert(fArray["second"] == "more contents");
	assertCompare( fArray.GetSize(), equal_to, lengthBefore );
	t_assert(fArray["third"] == Anything());
	// access of to a nonpresent slot returns an empty Anything
	t_assert(fArray.GetSize() == lengthBefore + 1);
	// and allocates the corresponding slot
	fArray["fourth"] = "new slot";
	lengthBefore = fArray.GetSize();
	t_assert(fArray["third"] == Anything());
	assertCompare( fArray.GetSize(), equal_to, lengthBefore );

} // KeyAccess0()

void AnythingKeyIndexTest::KeyAccess1()
{
	long i0, i1, i2, i3, i4;
	char idx0[3] = { 0 }, idx1[3] = { 0 }, idx2[3] = { 0 }, idx3[3] = { 0 },
									idx4[3] = { 0 };
	Anything any0;

	t_assert ( any0.GetSize() == 0 );

	// It does work because idx0, idx1, idx2, idx3, idx4 cannot be empty strings
	//----------------------------------------------------------------------------
	for (i0 = '0'; i0 < '5'; i0++) {
		long l0 = (long) (i0 - '0');
		idx0[0L] = (char) i0;
		// The element being defined is nor yet defined:  verify
		assertCompare( any0.IsDefined(idx0), equal_to, false );
		t_assert( any0.FindIndex(idx0) == -1 );
		assertCompare( any0.IsDefined(l0), equal_to, false );
		t_assert( any0.FindIndex(l0) == -1 );
		assertCompare( any0.Contains(idx0), equal_to, false );
		for (i1 = '0'; i1 < '5'; i1++) {
			long l1 = (long) (i1 - '0');
			idx1[0L] = (char) i1;
			// The element being defined is nor yet defined:  verify
			assertCompare( any0[idx0].IsDefined(idx1), equal_to, false );
			t_assert( any0[idx0].FindIndex(idx1) == -1 );
			assertCompare( any0[idx0].IsDefined(l1), equal_to, false );
			t_assert( any0[idx0].FindIndex(l1) == -1 );
			assertCompare( any0[idx0].Contains(idx1), equal_to, false );
			for (i2 = '0'; i2 < '5'; i2++) {
				long l2 = (long) (i2 - '0');
				idx2[0L] = (char) i2;
				// The element being defined is nor yet defined:  verify
				assertCompare( any0[idx0][idx1].IsDefined(idx2), equal_to, false );
				t_assert( any0[idx0][idx1].FindIndex(idx2) == -1 );
				assertCompare( any0[idx0][idx1].IsDefined(l2), equal_to, false );
				t_assert( any0[idx0][idx1].FindIndex(l2) == -1 );
				assertCompare( any0[idx0][idx1].Contains(idx2), equal_to, false );
				for (i3 = '0'; i3 < '5'; i3++) {
					long l3 = (long) (i3 - '0');
					idx3[0L] = (char) i3;
					// The element being defined is nor yet defined:  verify
					assertCompare( any0[idx0][idx1][idx2].IsDefined(idx3), equal_to, false );
					t_assert( any0[idx0][idx1][idx2].FindIndex(idx3) == -1 );
					assertCompare( any0[idx0][idx1][idx2].IsDefined(l3), equal_to, false );
					t_assert( any0[idx0][idx1][idx2].FindIndex(l3) == -1 );
					assertCompare( any0[idx0][idx1][idx2].Contains(idx3), equal_to, false );
					for (i4 = '0'; i4 < '5'; i4++) {
						long l4 = (long) (i4 - '0');
						idx4[0L] = (char) i4;
						// The element being defined is nor yet defined:  verify
						assertCompare( any0[idx0][idx1][idx2][idx3].IsDefined(idx4), equal_to, false );
						t_assert( any0[idx0][idx1][idx2][idx3].FindIndex(idx4) == -1 );
						assertCompare( any0[idx0][idx1][idx2][idx3].IsDefined(l4), equal_to, false );
						t_assert( any0[idx0][idx1][idx2][idx3].FindIndex(l4) == -1 );
						any0[idx0][idx1][idx2][idx3][idx4] = l0 + l1 + l2 + l3
															 + l4;
					}
				}
			}
		}
	}

	// Check the sizes of the dimensions
	t_assert ( any0.GetSize() == 5 );
	t_assert ( any0[0L].GetSize() == 5 );
	t_assert ( any0[0L][0L].GetSize() == 5 );
	t_assert ( any0[0L][0L][0L].GetSize() == 5 );
	t_assert ( any0[0L][0L][0L][0L].GetSize() == 5 );

	for (i0 = '0'; i0 < '5'; i0++) {
		long l0 = (long) (i0 - '0');
		idx0[0L] = (char) i0;
		// The elements are now defined:  verify
		assertCompare( any0.IsDefined(idx0), equal_to, true );
		assertCompare( any0.FindIndex(idx0), equal_to, l0 );
		//t_assert( any0.Contains(idx0) == false );
		// Check the slot-names
		t_assert( strcmp( any0.SlotName(l0), idx0 ) == 0 );
		// Check 'AT'
		t_assert( any0.At(idx0).At("0").At("0").At("0").At("0") == (l0) );
		t_assert( any0.At(l0).At("0").At(0L).At(0L).At(0L) == (l0) );
		// Check the name of the slot   PATRU

		String testString = any0.At(l0).At("0").At(0L).At(0L).SlotName(0L);
		assertEqual("0", testString);
		for (i1 = '0'; i1 < '5'; i1++) {
			long l1 = (long) (i1 - '0');
			idx1[0L] = (char) i1;
			// The elements are now defined:  verify
			assertCompare( any0[idx0].IsDefined(idx1), equal_to, true );
			assertCompare( any0[idx0].FindIndex(idx1), equal_to, l1 );
			assertCompare( any0[idx0].IsDefined(l1), equal_to, true );
			assertCompare( any0[idx0].FindIndex(l1), equal_to, l1 );
			//t_assert( any0[idx0].Contains(idx1) == false );
			// Check the slot-names
			t_assert( strcmp( any0.SlotName(l1), idx1 ) == 0 );
			// Check 'AT'
			t_assert( any0[idx0].At(idx1).At("0").At("0").At("0") == (l0) + (l1) );
			t_assert( any0[idx0].At(l1).At("0").At(0L).At(0L) == (l0) + (l1) );
			for (i2 = '0'; i2 < '5'; i2++) {
				long l2 = (long) (i2 - '0');
				idx2[0L] = (char) i2;
				// The elements are now defined:  verify
				assertCompare( any0[idx0][idx1].IsDefined(idx2), equal_to, true );
				assertCompare( any0[idx0][idx1].FindIndex(idx2), equal_to, l2 );
				//t_assert( any0[idx0][idx1].Contains(idx2) == false );
				// Check the slot-names
				t_assert( strcmp( any0[idx0][idx1].SlotName(l2), idx2 ) == 0 );
				// Check 'AT'
				t_assert( any0[idx0][idx1].At(idx2).At("0").At("0") == (l0) + (l1) + (l2) );
				t_assert( any0[idx0][idx1].At(l2).At(0L).At(0L) == (l0) + (l1) + (l2) );
				for (i3 = '0'; i3 < '5'; i3++) {
					long l3 = (long) (i3 - '0');
					idx3[0L] = (char) i3;
					// The elements are now defined:  verify
					t_assert( any0[idx0][idx1][idx2].IsDefined(idx3) == true );
					t_assert( any0[idx0][idx1][idx2].FindIndex(idx3) == l3 );
					//t_assert( any0[idx0][idx1][idx2].Contains(idx3) == false );
					// Check the slot-names
					t_assert( strcmp( any0[idx0][idx1][idx2].SlotName(l3), idx3 ) == 0 );
					// Check 'AT'
					t_assert( any0[idx0][idx1][idx2].At(idx3).At("0") == (l0) + (l1) + (l2) + (l3) );
					t_assert( any0[idx0][idx1][idx2].At(l3).At(0L) == (l0) + (l1) + (l2) + (l3) );
					for (i4 = '0'; i4 < '5'; i4++) {
						long l4 = (long) (i4 - '0');
						idx4[0L] = (char) i4;
						// The elements are now defined:  verify
						t_assert( any0[idx0][idx1][idx2][idx3].IsDefined(idx4) == true );
						t_assert( any0[idx0][idx1][idx2][idx3].FindIndex(idx4) == l4 );
						//t_assert( any0[idx0][idx1][idx2][idx3].Contains(idx4) == false );
						// Check the slot-names
						t_assert( strcmp( any0[idx0][idx1][idx2][idx3].SlotName(l4), idx4 ) == 0 );
						// Check 'AT'
						t_assert( any0[idx0][idx1][idx2][idx3][idx4] == (l0) + (l1) + (l2) + (l3) + (l4) );
						t_assert( any0[idx0][idx1][idx2][idx3].At(idx4) == (l0) + (l1) + (l2) + (l3) + (l4) );
						t_assert( any0[idx0][idx1][idx2][idx3].At(l4) == (l0) + (l1) + (l2) + (l3) + (l4) );
					}
				}
			}
		}
	}

	Anything anyHlp = any0["this"]["path"]["does"]["not"]["exist"], anyEmpty;

	t_assert ( any0.GetSize() == 6 ); // The first dimension has been incremented.  The other ones have not. WHY???
	t_assert ( any0[0L].GetSize() == 5 );
	t_assert ( any0[0L][0L].GetSize() == 5 );
	t_assert ( any0[0L][0L][0L].GetSize() == 5 );
	t_assert ( any0[0L][0L][0L][0L].GetSize() == 5 );
}
// KeyAccess1

void AnythingKeyIndexTest::KeyAccess2()
{
	// Anything used as a list
	Anything array;
	long i;

	for (i = 0; i < 1000; i++) {
		if (i < 300) {
			array.Append(i);
		} else if (i < 500) {
			array.Append((float) i + 0.12);
		} else if (i < 800) {
			array.Append(-1.0 * (float) i - 0.12);
		} else {
			array.Append("Test");
		}
	}

	for (i = 0; i < 1000; i++) {
		if (i < 300) {
			t_assert( array[i] == i );
			t_assert( array[i].GetType() == AnyLongType );
		} else if (i < 500) {
			t_assert( array[i] == (float)i + 0.12 );
			t_assert( array[i].GetType() == AnyDoubleType );
		} else if (i < 800) {
			t_assert( array[i] == -(float)i - 0.12 );
			t_assert( array[i].GetType() == AnyDoubleType );
		} else {
			t_assert( array[i] == "Test" );
			t_assert( array[i].GetType() == AnyCharPtrType );
		}
	}
}
// KeyAccess2

void AnythingKeyIndexTest::MixKeysAndArray()
{

	Anything any0, any_save, any_i0, any_i1;

	// Prepare:  Fill an anything
	any0 = AnythingKeyIndexTest::init5DimArray(2);
	any_save = any0;

	any_i0 = any0[0L];
	any_i1 = any0[1];

	t_assert( any0.GetSize() == 2 );
	any0.Append(233L);
	t_assert( any0.GetSize() == 3 );
	t_assert( any0[2].AsLong() == 233 );
	any0.Remove(0L);
	t_assert( any0.GetSize() == 2 );
	t_assert( any0[1].AsLong() == 233 );
	any0.Remove(0L);
	t_assert( any0.GetSize() == 1 );
	t_assert( any0[0L].AsLong() == 233 );
	any0.Remove(0L);
	t_assert( any0.GetSize() == 0 );

	any0["0"] = 12345L;
	any0["1"] = any_i1;
	any0["0"] = any_i0;

	t_assert( AnythingKeyIndexTest::check5DimArray( any0, any_save, 2 ) == true );
}

void AnythingKeyIndexTest::Recursive()
{
	Anything any0;

	any0.Append((long) 12);
	any0.Append((long) 23);
	any0.Append((long) 45);
	any0.Append(any0); // dangerous: may lead to a leak

	t_assert( any0.GetSize() == 4 );
	t_assert( any0[3].GetSize() == 4 );
	t_assert( any0[3][3].GetSize() == 4 );
	t_assert( any0[3][3][3].GetSize() == 4 );
	t_assert( any0[3][3][3][3].GetSize() == 4 );
	t_assert( any0[3][3][3][3][3].GetSize() == 4 );
	t_assert( any0[3][3][3][3][3][3].GetSize() == 4 );

	t_assert( any0[0L].AsLong() == 12 );
	t_assert( any0[1].AsLong() == 23 );
	t_assert( any0[2].AsLong() == 45 );
	t_assert( any0[3][0L].AsLong() == 12 );
	t_assert( any0[3][3][0L].AsLong() == 12 );
	t_assert( any0[3][3][3][0L].AsLong() == 12 );
	t_assert( any0[3][3][3][3][0L].AsLong() == 12 );
	t_assert( any0[3][3][3][3][3][0L].AsLong() == 12 );
	t_assert( any0[3][3][3][3][3][3][0L].AsLong() == 12 );
	t_assert( any0[3][3][3][3][3][3][1].AsLong() == 23 );
	t_assert( any0[3][3][3][3][3][3][2].AsLong() == 45 );

	any0.Append((long) 67);
	t_assert( any0[3][3][3][3][3][3][4].AsLong() == 67 );

	any0.Remove(3L); // break circular dependency to avoid leak

	Anything any1;

	any1["0"] = (long) 12;
	any1["1"] = (long) 23;
	any1["2"] = (long) 45;
	any1["3"] = any1; // dangerous: may lead to a leak

	t_assert( any1.GetSize() == 4 );
	t_assert( any1["3"].GetSize() == 4 );
	t_assert( any1["3"]["3"].GetSize() == 4 );
	t_assert( any1["3"]["3"]["3"].GetSize() == 4 );
	t_assert( any1["3"]["3"]["3"]["3"].GetSize() == 4 );
	t_assert( any1["3"]["3"]["3"]["3"]["3"].GetSize() == 4 );
	t_assert( any1["3"]["3"]["3"]["3"]["3"]["3"].GetSize() == 4 );

	t_assert( any1["0"].AsLong() == 12 );
	t_assert( any1["1"].AsLong() == 23 );
	t_assert( any1["2"].AsLong() == 45 );
	t_assert( any1["3"]["0"].AsLong() == 12 );
	t_assert( any1["3"]["3"]["0"].AsLong() == 12 );
	t_assert( any1["3"]["3"]["3"]["0"].AsLong() == 12 );
	t_assert( any1["3"]["3"]["3"]["3"]["0"].AsLong() == 12 );
	t_assert( any1["3"]["3"]["3"]["3"]["3"]["0"].AsLong() == 12 );
	t_assert( any1["3"]["3"]["3"]["3"]["3"]["3"]["0"].AsLong() == 12 );
	t_assert( any1["3"]["3"]["3"]["3"]["3"]["3"]["1"].AsLong() == 23 );
	t_assert( any1["3"]["3"]["3"]["3"]["3"]["3"]["2"].AsLong() == 45 );

	any1["4"] = (long) 67;
	t_assert( any1["3"]["3"]["3"]["3"]["3"]["3"]["4"].AsLong() == 67 );

	any1.Remove("3"); // break circular dependency to avoid leak
}

void AnythingKeyIndexTest::EmptyAccess0()
/* what: test access to Anythings with empty slots
 As a matter of fact the result of access with empty slots is
 undefined. The following tests just describes the behavior of
 the current implementation.
 (The behavior may seem weird in some places)
 */
{
	Anything test;

	int lengthBefore = test.GetSize();
	test[""] = 1L; // empty slotname, added anonymously
	assertEqual(lengthBefore + 1, test.GetSize());
	assertEqual(1, test[0L].AsLong(0));
	assertEqual(0, (long)test.SlotName(0));

	lengthBefore = test.GetSize();
	t_assert(test[""].GetType() == AnyNullType);
	// empty slotname still not present
	assertEqual(lengthBefore + 1, test.GetSize());
	// the above read access generated a new NullAnything
	t_assert(test[1].GetType() == AnyNullType);

	test["something"] = "Test";
	assertEqual("Test", test[2].AsCharPtr(""));
	lengthBefore = test.GetSize();
	t_assert(test[""].AsLong(1234) == 1234);
	// even after another assignment
	assertEqual(lengthBefore + 1, test.GetSize());
	// and again the number of slots increases
	t_assert(test[3].GetType() == AnyNullType);

	test[""] = "nothing";
	t_assert(test[""].AsString("really nothing") == "really nothing");

	test[""]["more"] = "adding more";
	t_assert(test[""]["more"].AsString("still nothing") == "still nothing");

	// read an Anything with an empty slotname
	String input = "{ /test \"some Text\" /\"\" \"more Text\" }";
	IStringStream is(input);
	t_assert(!test.Import(is));

} // EmptyAccess0

void AnythingKeyIndexTest::EmptyAccess1()
/* what: test access to Anythings with empty slots
 As a matter of fact the result of access with empty slots is
 undefined. The following tests just describes the behavior of
 the current implementation.
 */
{
	// the following tests only make sense if Coast is not compiled
	// with DEBUG enabled, since most of them are caught by an assert.

	Anything test;
	DummyIFAObj objTest("Test");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	long sizeBefore = test.GetSize();

	test[""] = 1L;
	t_assert( test.GetSize() == sizeBefore + 1 );
	sizeBefore = test.GetSize();
	t_assert( test[0L].AsLong() == 1 );
	t_assert ( test.IsDefined("") == false );
	t_assert( test.GetSize() == sizeBefore );

	long testNumber;
	testNumber = test[""].AsLong(3);
	assertEqual(3, testNumber);
	t_assert ( test.GetSize() == sizeBefore + 1 );
	sizeBefore = test.GetSize();

	testNumber = test[""][""].AsLong(5);
	assertEqual(5, testNumber);
	t_assert( test.GetSize() == sizeBefore + 1 );
	sizeBefore = test.GetSize();

	test[""] = 1L;
	t_assert( test[test.GetSize()-1L].AsBool() == true );
	t_assert( test.GetSize() == sizeBefore + 1 );
	sizeBefore = test.GetSize();

	t_assert ( test[""].GetSize() == 0 );
	t_assert( test.GetSize() == sizeBefore + 1 );
} // EmptyAccess1
