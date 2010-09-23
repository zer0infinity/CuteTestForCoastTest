/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnythingLookupTest.h"

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
#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//---- AnythingLookupTest ---------------------------------------------------------

AnythingLookupTest::AnythingLookupTest(TString tname) :
	TestCaseType(tname)
{
}

void AnythingLookupTest::setUp()
{
	StartTrace(AnythingLookupTest.setUp);
}

Test *AnythingLookupTest::suite()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnythingLookupTest, LookUp0Test);
	ADD_CASE(testSuite, AnythingLookupTest, LookUp1Test);
	ADD_CASE(testSuite, AnythingLookupTest, LookupPathByIndex);
	ADD_CASE(testSuite, AnythingLookupTest, EmptyLookup);
	ADD_CASE(testSuite, AnythingLookupTest, invPathLookup);
	ADD_CASE(testSuite, AnythingLookupTest, LookUpWithSpecialCharsTest);
	ADD_CASE(testSuite, AnythingLookupTest, LookupCaseSensitiveTest);
	return testSuite;
}

Anything AnythingLookupTest::init5DimArray(long anzElt)
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

void AnythingLookupTest::LookUp0Test()
{
	Anything parent;
	parent["Here"]["comes"]["some"] = "data";
	Anything derived;
	t_assert(parent.LookupPath(derived, "Here.comes"));
	assertEqual("data", derived["some"].AsString("x"));

	ROAnything roParent(parent);
	ROAnything roDerived;
	t_assert(roParent.LookupPath(roDerived, "Here.comes"));
	assertEqual("data", roDerived["some"].AsString("x"));

	derived["more"] = "new data";
	assertEqual("new data", parent["Here"]["comes"]["more"].AsString("x"));
}

void AnythingLookupTest::LookUp1Test()
{
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
			t_assert(any0.LookupPath( derived, path ));
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
			assertEqual(2 * ((l0) + (l1)) + 1, any0[idx0][idx1].AsLong());
		}
	}
}

void AnythingLookupTest::LookupPathByIndex()
{
	{
		// Index at the end
		Anything test;
		Anything result;
		ROAnything roTest;
		ROAnything roResult;

		// make some simple assertions, check if return value is correct for missing slots
		t_assert(!test.LookupPath(result, ":0"));
		assertAnyEqual(Anything(), result);
		t_assert(!roTest.LookupPath(roResult, ":0"));
		assertAnyEqual(Anything(), roResult);
		t_assert(!test.LookupPath(result, "190"));
		assertAnyEqual(Anything(), result);
		t_assert(!roTest.LookupPath(roResult, "190"));
		assertAnyEqual(Anything(), roResult);
		t_assert(!test.LookupPath(result, ":0.a"));
		assertAnyEqual(Anything(), result);
		t_assert(!roTest.LookupPath(roResult, ":0.a"));
		assertAnyEqual(Anything(), roResult);
		t_assert(!test.LookupPath(result, "190:0"));
		assertAnyEqual(Anything(), result);
		t_assert(!roTest.LookupPath(roResult, "190:0"));
		assertAnyEqual(Anything(), roResult);

		test.Append("a");
		test.Append("b");
		roTest = test;

		t_assert(test.LookupPath(result, ":0"));
		assertAnyEqual(Anything("a"), result);
		t_assert(roTest.LookupPath(roResult, ":0"));
		assertAnyEqual(Anything("a"), roResult);

		t_assert(test.LookupPath(result, ":1"));
		assertAnyEqual(Anything("b"), result);
		t_assert(roTest.LookupPath(roResult, ":1"));
		assertAnyEqual(Anything("b"), roResult);

		test["100"] = test;
		roTest = test;

		// test index access of third named slot
		t_assert(test.LookupPath(result, ":2:0"));
		assertAnyEqual(Anything("a"), result);
		t_assert(roTest.LookupPath(roResult, ":2:0"));
		assertAnyEqual(Anything("a"), roResult);
		t_assert(test.LookupPath(result, "100:0"));
		assertAnyEqual(Anything("a"), result);
		t_assert(roTest.LookupPath(roResult, "100:0"));
		assertAnyEqual(Anything("a"), roResult);

		t_assert(test.LookupPath(result, "100:1"));
		assertAnyEqual(Anything("b"), result);
		t_assert(roTest.LookupPath(roResult, "100:1"));
		assertAnyEqual(Anything("b"), roResult);

		test["100"] = test;
		roTest = test;

		t_assert(test.LookupPath(result, "100.100:0"));
		assertAnyEqual(Anything("a"), result);
		t_assert(roTest.LookupPath(roResult, "100.100:0"));
		assertAnyEqual(Anything("a"), roResult);

		t_assert(test.LookupPath(result, "100.100:1"));
		assertAnyEqual(Anything("b"), result);
		t_assert(roTest.LookupPath(roResult, "100.100:1"));
		assertAnyEqual(Anything("b"), roResult);
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

		t_assert(test.LookupPath(result, ":0.a1"));
		assertAnyEqual(Anything(1), result);
		t_assert(roTest.LookupPath(roResult, ":0.a1"));
		assertAnyEqual(Anything(1), roResult);

		t_assert(test.LookupPath(result, ":1.b2"));
		assertAnyEqual(Anything(4), result);
		t_assert(roTest.LookupPath(roResult, ":1.b2"));
		assertAnyEqual(Anything(4), roResult);

		test["100"] = test;
		roTest = test;

		t_assert(test.LookupPath(result, "100:0.b1"));
		assertAnyEqual(Anything(2), result);
		t_assert(roTest.LookupPath(roResult, "100:0.b1"));
		assertAnyEqual(Anything(2), roResult);

		t_assert(test.LookupPath(result, "100:1.a2"));
		assertAnyEqual(Anything(3), result);
		t_assert(roTest.LookupPath(roResult, "100:1.a2"));
		assertAnyEqual(Anything(3), roResult);

		test["100"] = test;
		roTest = test;

		t_assert(test.LookupPath(result, "100.100:0.a1"));
		assertAnyEqual(Anything(1), result);
		t_assert(roTest.LookupPath(roResult, "100.100:0.a1"));
		assertAnyEqual(Anything(1), roResult);

		t_assert(test.LookupPath(result, "100.100:1.b2"));
		assertAnyEqual(Anything(4), result);
		t_assert(roTest.LookupPath(roResult, "100.100:1.b2"));
		assertAnyEqual(Anything(4), roResult);

		{
			// the following test must fail because "100:1:1" is already the slot b2 so ".b2" is not possible anymore
			// use new anys because of required emptyness for comparison
			Anything result1;
			ROAnything roResult1;
			t_assert(!test.LookupPath(result1, "100:1:1.b2"));
			assertAnyEqual(Anything(), result1);
			t_assert(!roTest.LookupPath(roResult1, "100:1:1.b2"));
			assertAnyEqual(Anything(), roResult1);
		}

		test[0L]["c"] = test[1L];
		roTest = test;

		t_assert(test.LookupPath(result, "100:0.c.b2"));
		assertAnyEqual(Anything(4), result);
		t_assert(roTest.LookupPath(roResult, "100:0.c.b2"));
		assertAnyEqual(Anything(4), roResult);

		Anything leaf3;
		leaf3["a3"] = 5;
		leaf3["b3"] = 6;
		test[0L]["c"]["b2"].Append(leaf3);
		roTest = test;

		t_assert(test.LookupPath(result, "100:0.c.b2:0"));
		assertAnyEqual(Anything(4), result);
		t_assert(roTest.LookupPath(roResult, "100:0.c.b2:0"));
		assertAnyEqual(Anything(4), roResult);

		t_assert(test.LookupPath(result, "100:0.c.b2:1.a3"));
		assertAnyEqual(Anything(5), result);
		t_assert(roTest.LookupPath(roResult, "100:0.c.b2:1.a3"));
		assertAnyEqual(Anything(5), roResult);
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
	assertCompare( test.GetSize(), equal_to, lengthBefore );
	t_assert(test["foo"] == 1L);
	t_assert(test["bar"] == 2L);

	// do the test
	retVal = test.LookupPath(result, path);

	// post condition
	assertEqual(false, retVal);
	t_assert(result.IsNull());
	assertCompare( test.GetSize(), equal_to, 2L );
	t_assert(test["foo"] == 1L);
	t_assert(test["bar"] == 2L);

	retVal = test.LookupPath(result, "");

	// post condition
	assertEqual(false, retVal);
	t_assert(result.IsNull());
	assertCompare( test.GetSize(), equal_to, 2L );
	t_assert(test["foo"] == 1L);
	t_assert(test["bar"] == 2L);

	retVal = test.LookupPath(result, ".");

	// post condition
	assertEqual(false, retVal);
	t_assert(result.IsNull());
	assertCompare( test.GetSize(), equal_to, 2L );
	t_assert(test["foo"] == 1L);
	t_assert(test["bar"] == 2L);

	retVal = test.LookupPath(result, "..");

	// post condition
	assertEqual(false, retVal);
	t_assert(result.IsNull());
	assertCompare( test.GetSize(), equal_to, 2L );
	t_assert(test["foo"] == 1L);
	t_assert(test["bar"] == 2L);

	retVal = test.LookupPath(result, (const char *) 0);

	// post condition
	assertEqual(false, retVal);
	t_assert(result.IsNull());
	assertCompare( test.GetSize(), equal_to, 2L );
	t_assert(test["foo"] == 1L);
	t_assert(test["bar"] == 2L);
} // EmptyLookup

void AnythingLookupTest::invPathLookup()
{
	String path("Invalid path");
	Anything test;

	test["foo"] = 1L;
	test["bar"] = 2L;
	test["test"]["invalid"]["path"] = true;
	long lengthBefore = test.GetSize();

	bool retVal;
	Anything result;

	//pre condition
	assertCompare( test.GetSize(), equal_to, lengthBefore );
	t_assert(test["foo"] == 1L);
	t_assert(test["bar"] == 2L);

	// do the test
	retVal = test.LookupPath(result, path);

	// post condition
	assertEqual(false, retVal);
	t_assert(result.IsNull());
	assertCompare( test.GetSize(), equal_to, lengthBefore );
	t_assert(test["foo"] == 1L);
	t_assert(test["bar"] == 2L);

	intLookupPathCheck(test, "..test.invalid");
	intLookupPathCheck(test, ".test.invalid");
	intLookupPathCheck(test, "test..invalid");
	intLookupPathCheck(test, "test.");
	intLookupPathCheck(test, "test..");

} // invPathLookup

void AnythingLookupTest::intLookupPathCheck(Anything &test, const char *path)
{
	// do the test
	Anything result;
	bool retVal = test.LookupPath(result, path);
	assertEqualm(false, retVal, NotNull(path));
	t_assertm(result.IsNull(), NotNull(path));

	ROAnything roTest(test);
	ROAnything roResult;
	retVal = roTest.LookupPath(roResult, path);
	assertEqualm(false, retVal, NotNull(path));
	t_assertm(roResult.IsNull(), NotNull(path));
}

void AnythingLookupTest::LookUpWithSpecialCharsTest()
{
	Anything parent;
	parent["H�-r�"]["c�mes"]["$ome"] = "d�ta";
	assertEqual("d�ta", parent["H�-r�"]["c�mes"]["$ome"].AsString("x"));

	Anything derived;
	t_assert(parent.LookupPath(derived, "H�-r�.c�mes"));
	assertEqual("d�ta", derived["$ome"].AsString("x"));

	ROAnything roParent(parent);
	ROAnything roDerived;
	t_assert(roParent.LookupPath(roDerived, "H�-r�.c�mes"));
	assertEqual("d�ta", roDerived["$ome"].AsString("x"));

	derived["m�r�"] = "new data";
	assertEqual("new data", parent["H�-r�"]["c�mes"]["m�r�"].AsString("x"));

}

void AnythingLookupTest::LookupCaseSensitiveTest()
{
	Anything any;
	Anything res;
	any["ALL"] = "content";
	t_assert(any.LookupPath(res, "all") == 0);
}
