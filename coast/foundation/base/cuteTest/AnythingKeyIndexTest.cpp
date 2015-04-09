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

#include "AnythingKeyIndexTest.h"
#include "StringStream.h"
#include "IFAObject.h"
#include "Tracer.h"

AnythingKeyIndexTest::AnythingKeyIndexTest() {
	StartTrace(AnythingKeyIndexTest.setUp);
	fArray["slot"] = "contents";
	fArray["second"] = "more contents";
	fSequence.Append("A String");
	fSequence.Append("Another String");
}

class DummyIFAObj: public IFAObject {
public:
	DummyIFAObj(const char *) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) DummyIFAObj("dummy");
	}
};

void AnythingKeyIndexTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(AnythingKeyIndexTest, SimpleRemove));
	s.push_back(CUTE_SMEMFUN(AnythingKeyIndexTest, RemoveInvKeys));
	s.push_back(CUTE_SMEMFUN(AnythingKeyIndexTest, IndexAccess));
	s.push_back(CUTE_SMEMFUN(AnythingKeyIndexTest, KeyAccess0));
	s.push_back(CUTE_SMEMFUN(AnythingKeyIndexTest, KeyAccess1));
	s.push_back(CUTE_SMEMFUN(AnythingKeyIndexTest, KeyAccess2));
	s.push_back(CUTE_SMEMFUN(AnythingKeyIndexTest, MixKeysAndArray));
	s.push_back(CUTE_SMEMFUN(AnythingKeyIndexTest, Recursive));
	s.push_back(CUTE_SMEMFUN(AnythingKeyIndexTest, EmptyAccess0));
	s.push_back(CUTE_SMEMFUN(AnythingKeyIndexTest, EmptyAccess1));
}

Anything AnythingKeyIndexTest::init5DimArray(long anzElt) {
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

bool AnythingKeyIndexTest::check5DimArray(Anything &any0, Anything &any1, long anzElt) {
	long i0, i1;
	char idx0[3] = { 0 }, idx1[3] = { 0 };
	bool retVal = true;
	Anything any;

	for (i0 = '0'; i0 < anzElt + '0'; i0++) {
		idx0[0L] = (char) i0;
		if (any0[idx0].At("0").At("0").At("0").At("0") != any1[idx0].At("0").At("0").At("0").At("0")) {
			retVal = false;
		}
		for (i1 = '0'; i1 < anzElt + '0'; i1++) {
			idx1[0L] = (char) i1;
			if (any0[idx0][idx1].At("0").At("0").At("0") != any1[idx0][idx1].At("0").At("0").At("0")) {
				retVal = false;
			}
		}
	}
	return (retVal);
}

void AnythingKeyIndexTest::RemoveInvKeys() {
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
				ASSERT_EQUAL( sizeBefore, any0.GetSize() );
				ASSERT( any0.IsDefined( buf ) == false );
			}
		}
	}

	// Remove indexes which do not exist
	for (i0 = 1000000; i0 < 1002000; i0 += 137) {
		any0.Remove(i0);
		ASSERT_EQUAL( sizeBefore, any0.GetSize() );
		//		ASSERT( any0.IsDefined( i0 ) == false );
	}

	// Remove negative indexes
	for (i0 = -1000L; i0 < 0L; i0 += 13) {
		any0.Remove(i0); // alert, but no crash
		ASSERT_EQUAL( sizeBefore, any0.GetSize() );
		//		ASSERT( any0.IsDefined( i0 ) == false );
	}

	// Check if any0 till unchanged
	ASSERT( AnythingKeyIndexTest::check5DimArray(any0, any0, 2) == true );
}

void AnythingKeyIndexTest::SimpleRemove() {
	Anything simple;
	bool res;
	String resString;

	simple["one"] = "One";
	simple["two"] = "Two";
	simple["three"] = "Three";

	simple.Remove("two");
	res = simple.IsDefined("two");

	ASSERT_EQUAL(false, res);

	resString = simple.SlotName(0);
	ASSERT_EQUAL("one", resString);

	resString = simple.SlotName(1);
	ASSERT_EQUAL("three", resString);

	resString = simple.SlotName(2);
	ASSERT_EQUAL("", resString);
}

void AnythingKeyIndexTest::IndexAccess()
/* what: test access to Anythings through legal and illegal numeric indices*/
{
	long lengthBefore = fSequence.GetSize();

	ASSERT(fSequence[0L].AsString("") == "A String");
	ASSERT(fSequence[1].AsString("") == "Another String");
	ASSERT_EQUAL( fSequence.GetSize(), lengthBefore );
	int newMax = lengthBefore + 2;
	ASSERT(fSequence[newMax].AsString("") == "");
	ASSERT(fSequence.GetSize() == newMax + 1);
}

void AnythingKeyIndexTest::KeyAccess0()
/* what: test access to Anythings through regular string keys*/
{
	long lengthBefore = fArray.GetSize();

	ASSERT(fArray["slot"] == "contents");
	ASSERT(fArray["second"] == "more contents");
	ASSERT_EQUAL( fArray.GetSize(), lengthBefore );
	ASSERT(fArray["third"] == Anything());
	// access of to a nonpresent slot returns an empty Anything
	ASSERT(fArray.GetSize() == lengthBefore + 1);
	// and allocates the corresponding slot
	fArray["fourth"] = "new slot";
	lengthBefore = fArray.GetSize();
	ASSERT(fArray["third"] == Anything());
	ASSERT_EQUAL( fArray.GetSize(), lengthBefore );

} // KeyAccess0()

void AnythingKeyIndexTest::KeyAccess1() {
	long i0, i1, i2, i3, i4;
	char idx0[3] = { 0 }, idx1[3] = { 0 }, idx2[3] = { 0 }, idx3[3] = { 0 }, idx4[3] = { 0 };
	Anything any0;

	ASSERT ( any0.GetSize() == 0 );

	// It does work because idx0, idx1, idx2, idx3, idx4 cannot be empty strings
	//----------------------------------------------------------------------------
	for (i0 = '0'; i0 < '5'; i0++) {
		long l0 = (long) (i0 - '0');
		idx0[0L] = (char) i0;
		// The element being defined is nor yet defined:  verify
		ASSERT_EQUAL( any0.IsDefined(idx0), false );
		ASSERT( any0.FindIndex(idx0) == -1 );
		ASSERT_EQUAL( any0.IsDefined(l0), false );
		ASSERT( any0.FindIndex(l0) == -1 );
		ASSERT_EQUAL( any0.Contains(idx0), false );
		for (i1 = '0'; i1 < '5'; i1++) {
			long l1 = (long) (i1 - '0');
			idx1[0L] = (char) i1;
			// The element being defined is nor yet defined:  verify
			ASSERT_EQUAL( any0[idx0].IsDefined(idx1), false );
			ASSERT( any0[idx0].FindIndex(idx1) == -1 );
			ASSERT_EQUAL( any0[idx0].IsDefined(l1), false );
			ASSERT( any0[idx0].FindIndex(l1) == -1 );
			ASSERT_EQUAL( any0[idx0].Contains(idx1), false );
			for (i2 = '0'; i2 < '5'; i2++) {
				long l2 = (long) (i2 - '0');
				idx2[0L] = (char) i2;
				// The element being defined is nor yet defined:  verify
				ASSERT_EQUAL( any0[idx0][idx1].IsDefined(idx2), false );
				ASSERT( any0[idx0][idx1].FindIndex(idx2) == -1 );
				ASSERT_EQUAL( any0[idx0][idx1].IsDefined(l2), false );
				ASSERT( any0[idx0][idx1].FindIndex(l2) == -1 );
				ASSERT_EQUAL( any0[idx0][idx1].Contains(idx2), false );
				for (i3 = '0'; i3 < '5'; i3++) {
					long l3 = (long) (i3 - '0');
					idx3[0L] = (char) i3;
					// The element being defined is nor yet defined:  verify
					ASSERT_EQUAL( any0[idx0][idx1][idx2].IsDefined(idx3), false );
					ASSERT( any0[idx0][idx1][idx2].FindIndex(idx3) == -1 );
					ASSERT_EQUAL( any0[idx0][idx1][idx2].IsDefined(l3), false );
					ASSERT( any0[idx0][idx1][idx2].FindIndex(l3) == -1 );
					ASSERT_EQUAL( any0[idx0][idx1][idx2].Contains(idx3), false );
					for (i4 = '0'; i4 < '5'; i4++) {
						long l4 = (long) (i4 - '0');
						idx4[0L] = (char) i4;
						// The element being defined is nor yet defined:  verify
						ASSERT_EQUAL( any0[idx0][idx1][idx2][idx3].IsDefined(idx4), false );
						ASSERT( any0[idx0][idx1][idx2][idx3].FindIndex(idx4) == -1 );
						ASSERT_EQUAL( any0[idx0][idx1][idx2][idx3].IsDefined(l4), false );
						ASSERT( any0[idx0][idx1][idx2][idx3].FindIndex(l4) == -1 );
						any0[idx0][idx1][idx2][idx3][idx4] = l0 + l1 + l2 + l3 + l4;
					}
				}
			}
		}
	}

	// Check the sizes of the dimensions
	ASSERT ( any0.GetSize() == 5 );
	ASSERT ( any0[0L].GetSize() == 5 );
	ASSERT ( any0[0L][0L].GetSize() == 5 );
	ASSERT ( any0[0L][0L][0L].GetSize() == 5 );
	ASSERT ( any0[0L][0L][0L][0L].GetSize() == 5 );

	for (i0 = '0'; i0 < '5'; i0++) {
		long l0 = (long) (i0 - '0');
		idx0[0L] = (char) i0;
		// The elements are now defined:  verify
		ASSERT_EQUAL( any0.IsDefined(idx0), true );
		ASSERT_EQUAL( any0.FindIndex(idx0), l0 );
		//ASSERT( any0.Contains(idx0) == false );
		// Check the slot-names
		ASSERT( strcmp( any0.SlotName(l0), idx0 ) == 0 );
		// Check 'AT'
		ASSERT( any0.At(idx0).At("0").At("0").At("0").At("0") == (l0) );
		ASSERT( any0.At(l0).At("0").At(0L).At(0L).At(0L) == (l0) );
		// Check the name of the slot   PATRU

		String testString = any0.At(l0).At("0").At(0L).At(0L).SlotName(0L);
		ASSERT_EQUAL("0", testString);
		for (i1 = '0'; i1 < '5'; i1++) {
			long l1 = (long) (i1 - '0');
			idx1[0L] = (char) i1;
			// The elements are now defined:  verify
			ASSERT_EQUAL( any0[idx0].IsDefined(idx1), true );
			ASSERT_EQUAL( any0[idx0].FindIndex(idx1), l1 );
			ASSERT_EQUAL( any0[idx0].IsDefined(l1), true );
			ASSERT_EQUAL( any0[idx0].FindIndex(l1), l1 );
			//ASSERT( any0[idx0].Contains(idx1) == false );
			// Check the slot-names
			ASSERT( strcmp( any0.SlotName(l1), idx1 ) == 0 );
			// Check 'AT'
			ASSERT( any0[idx0].At(idx1).At("0").At("0").At("0") == (l0) + (l1) );
			ASSERT( any0[idx0].At(l1).At("0").At(0L).At(0L) == (l0) + (l1) );
			for (i2 = '0'; i2 < '5'; i2++) {
				long l2 = (long) (i2 - '0');
				idx2[0L] = (char) i2;
				// The elements are now defined:  verify
				ASSERT_EQUAL( any0[idx0][idx1].IsDefined(idx2), true );
				ASSERT_EQUAL( any0[idx0][idx1].FindIndex(idx2), l2 );
				//ASSERT( any0[idx0][idx1].Contains(idx2) == false );
				// Check the slot-names
				ASSERT( strcmp( any0[idx0][idx1].SlotName(l2), idx2 ) == 0 );
				// Check 'AT'
				ASSERT( any0[idx0][idx1].At(idx2).At("0").At("0") == (l0) + (l1) + (l2) );
				ASSERT( any0[idx0][idx1].At(l2).At(0L).At(0L) == (l0) + (l1) + (l2) );
				for (i3 = '0'; i3 < '5'; i3++) {
					long l3 = (long) (i3 - '0');
					idx3[0L] = (char) i3;
					// The elements are now defined:  verify
					ASSERT( any0[idx0][idx1][idx2].IsDefined(idx3) == true );
					ASSERT( any0[idx0][idx1][idx2].FindIndex(idx3) == l3 );
					//ASSERT( any0[idx0][idx1][idx2].Contains(idx3) == false );
					// Check the slot-names
					ASSERT( strcmp( any0[idx0][idx1][idx2].SlotName(l3), idx3 ) == 0 );
					// Check 'AT'
					ASSERT( any0[idx0][idx1][idx2].At(idx3).At("0") == (l0) + (l1) + (l2) + (l3) );
					ASSERT( any0[idx0][idx1][idx2].At(l3).At(0L) == (l0) + (l1) + (l2) + (l3) );
					for (i4 = '0'; i4 < '5'; i4++) {
						long l4 = (long) (i4 - '0');
						idx4[0L] = (char) i4;
						// The elements are now defined:  verify
						ASSERT( any0[idx0][idx1][idx2][idx3].IsDefined(idx4) == true );
						ASSERT( any0[idx0][idx1][idx2][idx3].FindIndex(idx4) == l4 );
						//ASSERT( any0[idx0][idx1][idx2][idx3].Contains(idx4) == false );
						// Check the slot-names
						ASSERT( strcmp( any0[idx0][idx1][idx2][idx3].SlotName(l4), idx4 ) == 0 );
						// Check 'AT'
						ASSERT( any0[idx0][idx1][idx2][idx3][idx4] == (l0) + (l1) + (l2) + (l3) + (l4) );
						ASSERT( any0[idx0][idx1][idx2][idx3].At(idx4) == (l0) + (l1) + (l2) + (l3) + (l4) );
						ASSERT( any0[idx0][idx1][idx2][idx3].At(l4) == (l0) + (l1) + (l2) + (l3) + (l4) );
					}
				}
			}
		}
	}

	Anything anyHlp = any0["this"]["path"]["does"]["not"]["exist"], anyEmpty;

	ASSERT ( any0.GetSize() == 6 ); // The first dimension has been incremented.  The other ones have not. WHY???
	ASSERT ( any0[0L].GetSize() == 5 );
	ASSERT ( any0[0L][0L].GetSize() == 5 );
	ASSERT ( any0[0L][0L][0L].GetSize() == 5 );
	ASSERT ( any0[0L][0L][0L][0L].GetSize() == 5 );
}
// KeyAccess1

void AnythingKeyIndexTest::KeyAccess2() {
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
			ASSERT( array[i] == i );
			ASSERT( array[i].GetType() == AnyLongType );
		} else if (i < 500) {
			ASSERT( array[i] == (float)i + 0.12 );
			ASSERT( array[i].GetType() == AnyDoubleType );
		} else if (i < 800) {
			ASSERT( array[i] == -(float)i - 0.12 );
			ASSERT( array[i].GetType() == AnyDoubleType );
		} else {
			ASSERT( array[i] == "Test" );
			ASSERT( array[i].GetType() == AnyCharPtrType );
		}
	}
}
// KeyAccess2

void AnythingKeyIndexTest::MixKeysAndArray() {

	Anything any0, any_save, any_i0, any_i1;

	// Prepare:  Fill an anything
	any0 = AnythingKeyIndexTest::init5DimArray(2);
	any_save = any0;

	any_i0 = any0[0L];
	any_i1 = any0[1];

	ASSERT( any0.GetSize() == 2 );
	any0.Append(233L);
	ASSERT( any0.GetSize() == 3 );
	ASSERT( any0[2].AsLong() == 233 );
	any0.Remove(0L);
	ASSERT( any0.GetSize() == 2 );
	ASSERT( any0[1].AsLong() == 233 );
	any0.Remove(0L);
	ASSERT( any0.GetSize() == 1 );
	ASSERT( any0[0L].AsLong() == 233 );
	any0.Remove(0L);
	ASSERT( any0.GetSize() == 0 );

	any0["0"] = 12345L;
	any0["1"] = any_i1;
	any0["0"] = any_i0;

	ASSERT( AnythingKeyIndexTest::check5DimArray( any0, any_save, 2 ) == true );
}

void AnythingKeyIndexTest::Recursive() {
	Anything any0;

	any0.Append((long) 12);
	any0.Append((long) 23);
	any0.Append((long) 45);
	any0.Append(any0); // dangerous: may lead to a leak

	ASSERT( any0.GetSize() == 4 );
	ASSERT( any0[3].GetSize() == 4 );
	ASSERT( any0[3][3].GetSize() == 4 );
	ASSERT( any0[3][3][3].GetSize() == 4 );
	ASSERT( any0[3][3][3][3].GetSize() == 4 );
	ASSERT( any0[3][3][3][3][3].GetSize() == 4 );
	ASSERT( any0[3][3][3][3][3][3].GetSize() == 4 );

	ASSERT( any0[0L].AsLong() == 12 );
	ASSERT( any0[1].AsLong() == 23 );
	ASSERT( any0[2].AsLong() == 45 );
	ASSERT( any0[3][0L].AsLong() == 12 );
	ASSERT( any0[3][3][0L].AsLong() == 12 );
	ASSERT( any0[3][3][3][0L].AsLong() == 12 );
	ASSERT( any0[3][3][3][3][0L].AsLong() == 12 );
	ASSERT( any0[3][3][3][3][3][0L].AsLong() == 12 );
	ASSERT( any0[3][3][3][3][3][3][0L].AsLong() == 12 );
	ASSERT( any0[3][3][3][3][3][3][1].AsLong() == 23 );
	ASSERT( any0[3][3][3][3][3][3][2].AsLong() == 45 );

	any0.Append((long) 67);
	ASSERT( any0[3][3][3][3][3][3][4].AsLong() == 67 );

	any0.Remove(3L); // break circular dependency to avoid leak

	Anything any1;

	any1["0"] = (long) 12;
	any1["1"] = (long) 23;
	any1["2"] = (long) 45;
	any1["3"] = any1; // dangerous: may lead to a leak

	ASSERT( any1.GetSize() == 4 );
	ASSERT( any1["3"].GetSize() == 4 );
	ASSERT( any1["3"]["3"].GetSize() == 4 );
	ASSERT( any1["3"]["3"]["3"].GetSize() == 4 );
	ASSERT( any1["3"]["3"]["3"]["3"].GetSize() == 4 );
	ASSERT( any1["3"]["3"]["3"]["3"]["3"].GetSize() == 4 );
	ASSERT( any1["3"]["3"]["3"]["3"]["3"]["3"].GetSize() == 4 );

	ASSERT( any1["0"].AsLong() == 12 );
	ASSERT( any1["1"].AsLong() == 23 );
	ASSERT( any1["2"].AsLong() == 45 );
	ASSERT( any1["3"]["0"].AsLong() == 12 );
	ASSERT( any1["3"]["3"]["0"].AsLong() == 12 );
	ASSERT( any1["3"]["3"]["3"]["0"].AsLong() == 12 );
	ASSERT( any1["3"]["3"]["3"]["3"]["0"].AsLong() == 12 );
	ASSERT( any1["3"]["3"]["3"]["3"]["3"]["0"].AsLong() == 12 );
	ASSERT( any1["3"]["3"]["3"]["3"]["3"]["3"]["0"].AsLong() == 12 );
	ASSERT( any1["3"]["3"]["3"]["3"]["3"]["3"]["1"].AsLong() == 23 );
	ASSERT( any1["3"]["3"]["3"]["3"]["3"]["3"]["2"].AsLong() == 45 );

	any1["4"] = (long) 67;
	ASSERT( any1["3"]["3"]["3"]["3"]["3"]["3"]["4"].AsLong() == 67 );

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
	ASSERT_EQUAL(lengthBefore + 1, test.GetSize());
	ASSERT_EQUAL(1, test[0L].AsLong(0));
	ASSERT_EQUAL(0, (long)test.SlotName(0));

	lengthBefore = test.GetSize();
	ASSERT(test[""].GetType() == AnyNullType);
	// empty slotname still not present
	ASSERT_EQUAL(lengthBefore + 1, test.GetSize());
	// the above read access generated a new NullAnything
	ASSERT(test[1].GetType() == AnyNullType);

	test["something"] = "Test";
	ASSERT_EQUAL("Test", test[2].AsCharPtr(""));
	lengthBefore = test.GetSize();
	ASSERT(test[""].AsLong(1234) == 1234);
	// even after another assignment
	ASSERT_EQUAL(lengthBefore + 1, test.GetSize());
	// and again the number of slots increases
	ASSERT(test[3].GetType() == AnyNullType);

	test[""] = "nothing";
	ASSERT(test[""].AsString("really nothing") == "really nothing");

	test[""]["more"] = "adding more";
	ASSERT(test[""]["more"].AsString("still nothing") == "still nothing");

	// read an Anything with an empty slotname
	String input = "{ /test \"some Text\" /\"\" \"more Text\" }";
	IStringStream is(input);
	ASSERT(!test.Import(is));

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
	ASSERT( test.GetSize() == sizeBefore + 1 );
	sizeBefore = test.GetSize();
	ASSERT( test[0L].AsLong() == 1 );
	ASSERT ( test.IsDefined("") == false );
	ASSERT( test.GetSize() == sizeBefore );

	long testNumber;
	testNumber = test[""].AsLong(3);
	ASSERT_EQUAL(3, testNumber);
	ASSERT ( test.GetSize() == sizeBefore + 1 );
	sizeBefore = test.GetSize();

	testNumber = test[""][""].AsLong(5);
	ASSERT_EQUAL(5, testNumber);
	ASSERT( test.GetSize() == sizeBefore + 1 );
	sizeBefore = test.GetSize();

	test[""] = 1L;
	ASSERT( test[test.GetSize()-1L].AsBool() == true );
	ASSERT( test.GetSize() == sizeBefore + 1 );
	sizeBefore = test.GetSize();

	ASSERT ( test[""].GetSize() == 0 );
	ASSERT( test.GetSize() == sizeBefore + 1 );
} // EmptyAccess1
