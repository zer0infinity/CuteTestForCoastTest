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

#include "AnythingLookupTest.h"

void AnythingLookupTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(AnythingLookupTest, LookUp0Test));
	s.push_back(CUTE_SMEMFUN(AnythingLookupTest, LookUp1Test));
	s.push_back(CUTE_SMEMFUN(AnythingLookupTest, LookupPathByIndex));
	s.push_back(CUTE_SMEMFUN(AnythingLookupTest, EmptyLookup));
	s.push_back(CUTE_SMEMFUN(AnythingLookupTest, invPathLookup));
	s.push_back(CUTE_SMEMFUN(AnythingLookupTest, LookUpWithSpecialCharsTest));
	s.push_back(CUTE_SMEMFUN(AnythingLookupTest, LookupCaseSensitiveTest));
}

Anything AnythingLookupTest::init5DimArray(long anzElt) {
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

void AnythingLookupTest::LookUp0Test() {
	Anything parent;
	parent["Here"]["comes"]["some"] = "data";
	Anything derived;
	ASSERT(parent.LookupPath(derived, "Here.comes"));
	ASSERT_EQUAL("data", derived["some"].AsString("x"));

	ROAnything roParent(parent);
	ROAnything roDerived;
	ASSERT(roParent.LookupPath(roDerived, "Here.comes"));
	ASSERT_EQUAL("data", roDerived["some"].AsString("x"));

	derived["more"] = "new data";
	ASSERT_EQUAL("new data", parent["Here"]["comes"]["more"].AsString("x"));
}

void AnythingLookupTest::LookUp1Test() {
	long i0, i1;
	char idx0[3] = { 0 }, idx1[3] = { 0 };
	String path;
	Anything any0, any1, anyTest, derived;

	// Init 'any0'
	any0 = AnythingLookupTest::init5DimArray(5);

	// Reinit 'any0' using LookupPath
	for (i0 = '0'; i0 < '5'; i0++) {
		long l0 = (long) (i0 - '0');
		idx0[0L] = (char) i0;
		for (i1 = '0'; i1 < '5'; i1++) {
			long l1 = (long) (i1 - '0');
			idx1[0L] = (char) i1;
			path = idx0;
			ASSERT(any0.LookupPath( derived, path ));
			derived[idx1] = 2 * ((l0) + (l1)) + 1;
		}
	}

	// Test if reinit was OK
	for (i0 = '0'; i0 < '5'; i0++) {
		long l0 = (long) (i0 - '0');
		idx0[0L] = (char) i0;
		for (i1 = '0'; i1 < '5'; i1++) {
			long l1 = (long) (i1 - '0');
			idx1[0L] = (char) i1;
			ASSERT_EQUAL(2 * ((l0) + (l1)) + 1, any0[idx0][idx1].AsLong());
		}
	}
}

void AnythingLookupTest::LookupPathByIndex() {
	{
		// Index at the end
		Anything test;
		Anything result;
		ROAnything roTest;
		ROAnything roResult;

		// make some simple assertions, check if return value is correct for missing slots
		ASSERT(!test.LookupPath(result, ":0"));
		ASSERT_EQUAL(Anything(), result);
		ASSERT(!roTest.LookupPath(roResult, ":0"));
		ASSERT_EQUAL(Anything(), roResult);
		ASSERT(!test.LookupPath(result, "190"));
		ASSERT_EQUAL(Anything(), result);
		ASSERT(!roTest.LookupPath(roResult, "190"));
		ASSERT_EQUAL(Anything(), roResult);
		ASSERT(!test.LookupPath(result, ":0.a"));
		ASSERT_EQUAL(Anything(), result);
		ASSERT(!roTest.LookupPath(roResult, ":0.a"));
		ASSERT_EQUAL(Anything(), roResult);
		ASSERT(!test.LookupPath(result, "190:0"));
		ASSERT_EQUAL(Anything(), result);
		ASSERT(!roTest.LookupPath(roResult, "190:0"));
		ASSERT_EQUAL(Anything(), roResult);

		test.Append("a");
		test.Append("b");
		roTest = test;

		ASSERT(test.LookupPath(result, ":0"));
		ASSERT_EQUAL(Anything("a"), result);
		ASSERT(roTest.LookupPath(roResult, ":0"));
		ASSERT_EQUAL(Anything("a"), roResult);

		ASSERT(test.LookupPath(result, ":1"));
		ASSERT_EQUAL(Anything("b"), result);
		ASSERT(roTest.LookupPath(roResult, ":1"));
		ASSERT_EQUAL(Anything("b"), roResult);

		test["100"] = test;
		roTest = test;

		// test index access of third named slot
		ASSERT(test.LookupPath(result, ":2:0"));
		ASSERT_EQUAL(Anything("a"), result);
		ASSERT(roTest.LookupPath(roResult, ":2:0"));
		ASSERT_EQUAL(Anything("a"), roResult);
		ASSERT(test.LookupPath(result, "100:0"));
		ASSERT_EQUAL(Anything("a"), result);
		ASSERT(roTest.LookupPath(roResult, "100:0"));
		ASSERT_EQUAL(Anything("a"), roResult);

		ASSERT(test.LookupPath(result, "100:1"));
		ASSERT_EQUAL(Anything("b"), result);
		ASSERT(roTest.LookupPath(roResult, "100:1"));
		ASSERT_EQUAL(Anything("b"), roResult);

		test["100"] = test;
		roTest = test;

		ASSERT(test.LookupPath(result, "100.100:0"));
		ASSERT_EQUAL(Anything("a"), result);
		ASSERT(roTest.LookupPath(roResult, "100.100:0"));
		ASSERT_EQUAL(Anything("a"), roResult);

		ASSERT(test.LookupPath(result, "100.100:1"));
		ASSERT_EQUAL(Anything("b"), result);
		ASSERT(roTest.LookupPath(roResult, "100.100:1"));
		ASSERT_EQUAL(Anything("b"), roResult);
	}
	{
		// Index between
		Anything test;
		Anything result;
		Anything leaf1, leaf2;
		ROAnything roTest;
		ROAnything roResult;

		leaf1["a1"] = 1;
		leaf1["b1"] = 2;

		leaf2["a2"] = 3;
		leaf2["b2"] = 4;

		test.Append(leaf1);
		test.Append(leaf2);
		roTest = test;

		ASSERT(test.LookupPath(result, ":0.a1"));
		ASSERT_EQUAL(Anything(1), result);
		ASSERT(roTest.LookupPath(roResult, ":0.a1"));
		ASSERT_EQUAL(Anything(1), roResult);

		ASSERT(test.LookupPath(result, ":1.b2"));
		ASSERT_EQUAL(Anything(4), result);
		ASSERT(roTest.LookupPath(roResult, ":1.b2"));
		ASSERT_EQUAL(Anything(4), roResult);

		test["100"] = test;
		roTest = test;

		ASSERT(test.LookupPath(result, "100:0.b1"));
		ASSERT_EQUAL(Anything(2), result);
		ASSERT(roTest.LookupPath(roResult, "100:0.b1"));
		ASSERT_EQUAL(Anything(2), roResult);

		ASSERT(test.LookupPath(result, "100:1.a2"));
		ASSERT_EQUAL(Anything(3), result);
		ASSERT(roTest.LookupPath(roResult, "100:1.a2"));
		ASSERT_EQUAL(Anything(3), roResult);

		test["100"] = test;
		roTest = test;

		ASSERT(test.LookupPath(result, "100.100:0.a1"));
		ASSERT_EQUAL(Anything(1), result);
		ASSERT(roTest.LookupPath(roResult, "100.100:0.a1"));
		ASSERT_EQUAL(Anything(1), roResult);

		ASSERT(test.LookupPath(result, "100.100:1.b2"));
		ASSERT_EQUAL(Anything(4), result);
		ASSERT(roTest.LookupPath(roResult, "100.100:1.b2"));
		ASSERT_EQUAL(Anything(4), roResult);

		{
			// the following test must fail because "100:1:1" is already the slot b2 so ".b2" is not possible anymore
			// use new anys because of required emptyness for comparison
			Anything result1;
			ROAnything roResult1;
			ASSERT(!test.LookupPath(result1, "100:1:1.b2"));
			ASSERT_EQUAL(Anything(), result1);
			ASSERT(!roTest.LookupPath(roResult1, "100:1:1.b2"));
			ASSERT_EQUAL(Anything(), roResult1);
		}

		test[0L]["c"] = test[1L];
		roTest = test;

		ASSERT(test.LookupPath(result, "100:0.c.b2"));
		ASSERT_EQUAL(Anything(4), result);
		ASSERT(roTest.LookupPath(roResult, "100:0.c.b2"));
		ASSERT_EQUAL(Anything(4), roResult);

		Anything leaf3;
		leaf3["a3"] = 5;
		leaf3["b3"] = 6;
		test[0L]["c"]["b2"].Append(leaf3);
		roTest = test;

		ASSERT(test.LookupPath(result, "100:0.c.b2:0"));
		ASSERT_EQUAL(Anything(4), result);
		ASSERT(roTest.LookupPath(roResult, "100:0.c.b2:0"));
		ASSERT_EQUAL(Anything(4), roResult);

		ASSERT(test.LookupPath(result, "100:0.c.b2:1.a3"));
		ASSERT_EQUAL(Anything(5), result);
		ASSERT(roTest.LookupPath(roResult, "100:0.c.b2:1.a3"));
		ASSERT_EQUAL(Anything(5), roResult);
	}
}

void AnythingLookupTest::EmptyLookup()
/* what: test lookupPath with empty path
 */
{
	String path; // this path is intentionally left empty
	Anything test;

	test["foo"] = 1L;
	test["bar"] = 2L;
	long lengthBefore = test.GetSize();

	bool retVal;
	Anything result;

	//pre condition
	ASSERT_EQUAL( test.GetSize(), lengthBefore );
	ASSERT(test["foo"] == 1L);
	ASSERT(test["bar"] == 2L);

	// do the test
	retVal = test.LookupPath(result, path);

	// post condition
	ASSERT_EQUAL(false, retVal);
	ASSERT(result.IsNull());
	ASSERT_EQUAL( test.GetSize(), 2L );
	ASSERT(test["foo"] == 1L);
	ASSERT(test["bar"] == 2L);

	retVal = test.LookupPath(result, "");

	// post condition
	ASSERT_EQUAL(false, retVal);
	ASSERT(result.IsNull());
	ASSERT_EQUAL( test.GetSize(), 2L );
	ASSERT(test["foo"] == 1L);
	ASSERT(test["bar"] == 2L);

	retVal = test.LookupPath(result, ".");

	// post condition
	ASSERT_EQUAL(false, retVal);
	ASSERT(result.IsNull());
	ASSERT_EQUAL( test.GetSize(), 2L );
	ASSERT(test["foo"] == 1L);
	ASSERT(test["bar"] == 2L);

	retVal = test.LookupPath(result, "..");

	// post condition
	ASSERT_EQUAL(false, retVal);
	ASSERT(result.IsNull());
	ASSERT_EQUAL( test.GetSize(), 2L );
	ASSERT(test["foo"] == 1L);
	ASSERT(test["bar"] == 2L);

	retVal = test.LookupPath(result, (const char *) 0);

	// post condition
	ASSERT_EQUAL(false, retVal);
	ASSERT(result.IsNull());
	ASSERT_EQUAL( test.GetSize(), 2L );
	ASSERT(test["foo"] == 1L);
	ASSERT(test["bar"] == 2L);
} // EmptyLookup

void AnythingLookupTest::invPathLookup() {
	String path("Invalid path");
	Anything test;

	test["foo"] = 1L;
	test["bar"] = 2L;
	test["test"]["invalid"]["path"] = true;
	long lengthBefore = test.GetSize();

	bool retVal;
	Anything result;

	//pre condition
	ASSERT_EQUAL( test.GetSize(), lengthBefore );
	ASSERT(test["foo"] == 1L);
	ASSERT(test["bar"] == 2L);

	// do the test
	retVal = test.LookupPath(result, path);

	// post condition
	ASSERT_EQUAL(false, retVal);
	ASSERT(result.IsNull());
	ASSERT_EQUAL( test.GetSize(), lengthBefore );
	ASSERT(test["foo"] == 1L);
	ASSERT(test["bar"] == 2L);

	intLookupPathCheck(test, "..test.invalid");
	intLookupPathCheck(test, ".test.invalid");
	intLookupPathCheck(test, "test..invalid");
	intLookupPathCheck(test, "test.");
	intLookupPathCheck(test, "test..");

} // invPathLookup

void AnythingLookupTest::intLookupPathCheck(Anything &test, const char *path) {
	// do the test
	Anything result;
	bool retVal = test.LookupPath(result, path);
	ASSERT_EQUALM( NotNull(path), 1, retVal);
	ASSERTM( NotNull(path),result.IsNull());

	ROAnything roTest(test);
	ROAnything roResult;
	retVal = roTest.LookupPath(roResult, path);
	ASSERT_EQUALM( NotNull(path), 1, retVal);
	ASSERTM( NotNull(path),roResult.IsNull());
}

void AnythingLookupTest::LookUpWithSpecialCharsTest() {
	Anything parent;
	parent["H�-r�"]["c�mes"]["$ome"] = "d�ta";
	ASSERT_EQUAL("d�ta", parent["H�-r�"]["c�mes"]["$ome"].AsString("x"));

	Anything derived;
	ASSERT(parent.LookupPath(derived, "H�-r�.c�mes"));
	ASSERT_EQUAL("d�ta", derived["$ome"].AsString("x"));

	ROAnything roParent(parent);
	ROAnything roDerived;
	ASSERT(roParent.LookupPath(roDerived, "H�-r�.c�mes"));
	ASSERT_EQUAL("d�ta", roDerived["$ome"].AsString("x"));

	derived["m�r�"] = "new data";
	ASSERT_EQUAL("new data", parent["H�-r�"]["c�mes"]["m�r�"].AsString("x"));

}

void AnythingLookupTest::LookupCaseSensitiveTest() {
	Anything any;
	Anything res;
	any["ALL"] = "content";
	ASSERT(any.LookupPath(res, "all") == 0);
}
