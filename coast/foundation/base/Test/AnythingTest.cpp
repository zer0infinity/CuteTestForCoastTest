/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnythingTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "IFAObject.h"
#include "SystemFile.h"
#include "Dbg.h"
#include "AnyIterators.h"
#include "SystemLog.h"

using namespace Coast;

//--- c-library modules used ---------------------------------------------------
#include <cstring>

//---- AnythingTest ---------------------------------------------------------

AnythingTest::AnythingTest(TString tname) :
	TestCaseType(tname)
{
}

void AnythingTest::setUp()
{
	StartTrace(AnythingTest.setUp);
	if (!System::LoadConfigFile(fConfig, "AnythingTest", "any")) {
		assertEqual( "'read AnythingTest.any'", "'could not read AnythingTest.any'" );
	}
	fQuery = fConfig["Queries"][name()];
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

Test *AnythingTest::suite()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AnythingTest, TypeTest);
	ADD_CASE(testSuite, AnythingTest, SuccessiveAssignments);
	ADD_CASE(testSuite, AnythingTest, BinaryBufOutput);
	ADD_CASE(testSuite, AnythingTest, roConversion);
	ADD_CASE(testSuite, AnythingTest, operatorAssignemnt);
	ADD_CASE(testSuite, AnythingTest, appendTest);
	ADD_CASE(testSuite, AnythingTest, String2LongConversion);
	ADD_CASE(testSuite, AnythingTest, AsCharPtrBufLen);
	ADD_CASE(testSuite, AnythingTest, RefCount);

	ADD_CASE(testSuite, AnythingTest, AnythingLeafIteratorTest);
	ADD_CASE(testSuite, AnythingTest, SlotFinderTest);
	ADD_CASE(testSuite, AnythingTest, SlotCopierTest);
	ADD_CASE(testSuite, AnythingTest, SlotPutterTest);
	ADD_CASE(testSuite, AnythingTest, SlotPutterAppendTest);
	ADD_CASE(testSuite, AnythingTest, SlotnameSorterTest);
	ADD_CASE(testSuite, AnythingTest, SlotCleanerTest);
	return testSuite;
}

/*============================================================================================================*/
/*  C L A S S M E T H O D S  (helpmethods)       Beginn                                                       */
/*============================================================================================================*/
Anything AnythingTest::init5DimArray(long anzElt)
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

bool AnythingTest::check5DimArray(Anything &any0, Anything &any1, long anzElt)
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

/*======================================================================================================================*/
/*  C L A S S M E T H O D S  (helpmethods)       Ende                                                                   */
/*======================================================================================================================*/

void AnythingTest::TypeTest()
{
	Anything TypeChange;
	Anything ArrayTypeTest;
	Anything KeyTypeTest;
	Anything AnyLong(5L);
	Anything AnyVoidBuff((void *) "Bytebuffer", 10);
	Anything SubAnything;
	String MyString("Another String");

	// Simple Types
	t_assert( TypeChange.GetType() == AnyNullType );

	TypeChange = 12L;
	t_assert( TypeChange.GetType() == AnyLongType );

	TypeChange = (char *) "Ein String";
	t_assert( TypeChange.GetType() == AnyCharPtrType );

	TypeChange = (long) 34;
	t_assert( TypeChange.GetType() == AnyLongType );

	TypeChange = (double) 300.44;
	t_assert( TypeChange.GetType() == AnyDoubleType );

	TypeChange = (long) 56;
	t_assert( TypeChange.GetType() == AnyLongType );

	TypeChange = MyString;
	t_assert( TypeChange.GetType() == AnyCharPtrType );

	TypeChange = (float) 1.37;
	t_assert( TypeChange.GetType() == AnyDoubleType );

	TypeChange = AnyLong;
	t_assert( TypeChange.GetType() == AnyLongType );

	// Array
	ArrayTypeTest[""] = ("Noch ein String");
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[""] = (12L);
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );

	ArrayTypeTest[""] = ((long) 34);
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 2].GetType() == AnyLongType );

	ArrayTypeTest[""] = ((long) 56);
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 2].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 3].GetType() == AnyLongType );

	ArrayTypeTest[""] = ((float) 98.765);
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 2].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 3].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 4].GetType() == AnyDoubleType );

	ArrayTypeTest[""] = ((double) 12.345);
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 2].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 3].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 4].GetType() == AnyDoubleType );
	t_assert( ArrayTypeTest[ 5].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = 12L;
	t_assert( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = (char *) "Ein String";
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[0L] = 34L;
	t_assert( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = (double) 300.44;
	t_assert( ArrayTypeTest[0L].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = (long) 56;
	t_assert( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = (float) 1.37;
	t_assert( ArrayTypeTest[0L].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = AnyLong;
	t_assert( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = MyString;
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );

	SubAnything = ArrayTypeTest;
	ArrayTypeTest.Append(SubAnything); // this will lead to a leak!
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 2].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 3].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 4].GetType() == AnyDoubleType );
	t_assert( ArrayTypeTest[ 5].GetType() == AnyDoubleType );
	t_assert( ArrayTypeTest[ 6].GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[6][ 1].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[6][ 2].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[6][ 3].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[6][ 4].GetType() == AnyDoubleType );
	t_assert( ArrayTypeTest[6][ 5].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = 12L;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest[6][0L] = (char *) "Ein String";
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[6][0L] = (long) 34;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest[6][0L] = (double) 300.44;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyDoubleType );

	ArrayTypeTest[6][0L] = (long) 56;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest[6][0L] = MyString;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[6][0L] = (float) 1.37;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyDoubleType );

	ArrayTypeTest[6][0L] = AnyLong;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest.Remove(6L); // break circular dependency to avoid leak

	// Array
	KeyTypeTest["0"] = "Noch ein String";
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["1"] = (12L);
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );

	KeyTypeTest["2"] = ((long) 34);
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["2"].GetType() == AnyLongType );

	KeyTypeTest["3"] = ((long) 56);
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["2"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["3"].GetType() == AnyLongType );

	KeyTypeTest["4"] = ((float) 98.765);
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["2"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["3"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["4"].GetType() == AnyDoubleType );

	KeyTypeTest["5"] = ((double) 12.345);
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["2"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["3"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["4"].GetType() == AnyDoubleType );
	t_assert( KeyTypeTest["5"].GetType() == AnyDoubleType );

	KeyTypeTest["0"] = 12L;
	t_assert( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = (char *) "Ein String";
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["0"] = (long) 34;
	t_assert( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = (double) 300.44;
	t_assert( KeyTypeTest["0"].GetType() == AnyDoubleType );

	KeyTypeTest["0"] = (long) 56;
	t_assert( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = (float) 1.37;
	t_assert( KeyTypeTest["0"].GetType() == AnyDoubleType );

	KeyTypeTest["0"] = AnyLong;
	t_assert( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = MyString;
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );

	SubAnything = KeyTypeTest;
	KeyTypeTest["6"] = (SubAnything);
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["2"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["3"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["4"].GetType() == AnyDoubleType );
	t_assert( KeyTypeTest["5"].GetType() == AnyDoubleType );
	t_assert( KeyTypeTest["6"].GetType() == AnyArrayType );
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["6"]["1"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["6"]["2"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["6"]["3"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["6"]["4"].GetType() == AnyDoubleType );
	t_assert( KeyTypeTest["6"]["5"].GetType() == AnyDoubleType );

	KeyTypeTest["6"]["0"] = 12L;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest["6"]["0"] = (char *) "Ein String";
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["6"]["0"] = (long) 34;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest["6"]["0"] = (double) 300.44;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyDoubleType );

	KeyTypeTest["6"]["0"] = (long) 56;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest["6"]["0"] = MyString;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["6"]["0"] = (float) 1.37;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyDoubleType );

	KeyTypeTest["6"]["0"] = AnyLong;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest.Remove("6"); // break circular dependency to avoid leak
}

void AnythingTest::SuccessiveAssignments()
// some random assignments with a strange outcome
{
	Anything a = "Test";
	t_assert(!a.IsDefined("Test"));
	assertEqual("Test", a.AsCharPtr("nothing"));

	if (a.IsDefined("Test")) {
		a["Test"] = "Was ist hier los ?";
	}

	Anything b = "Test";
	assertEqual("Test", b.AsCharPtr("nothing"));
	Anything c = b["Test"];
	t_assert( b.IsDefined("Test") );
	t_assert( c.IsNull() );
} // SuccessiveAssignments

void AnythingTest::operatorAssignemnt()
{
	boolOperatorAssign();
	intOperatorAssign();
	ifaObjectOperatorAssign();
}

void AnythingTest::appendTest()
{
	Anything sub;
	sub["X"] = "x";
	sub["Y"] = "y";

	Anything a;
	a.Append(sub);
	Anything expected;
	expected[0L] = sub;
	assertAnyEqual(expected, a);

	a.Append(sub);
	expected[1L] = sub;
	assertAnyEqual(expected, a);

	Anything b = "Test"; // AnyStringImpl
	b.Append(sub); // -> AnyArrayImpl
	expected[0L] = "Test";
	assertAnyEqual(expected, b);

	b["NewKey"].Append(sub);
	expected["NewKey"][0L] = sub;
	assertAnyEqual(expected, b);
}

void AnythingTest::boolOperatorAssign()
{
	// test bool operator
	bool b = true;

	Anything boolAny = b;
	assertEqual((long)(b), (long) boolAny.AsBool(false));

	b = false;
	boolAny = b;
	assertEqual((long)(b), (long) boolAny.AsBool(true));

	boolAny = 1;
	assertEqual((long)(true), (long) boolAny.AsBool(false));

	boolAny = 0;
	assertEqual((long)(false), (long) boolAny.AsBool(false));

	// some quirky cases
	long c = -1;
	long c1 = 1;
	boolAny = ((IFAObject *) c);
	assertEqual((long)(false), (long) boolAny.AsBool(false));

	boolAny = ((IFAObject *) c1);
	assertEqual((long)(false), (long) boolAny.AsBool(false));
}

void AnythingTest::intOperatorAssign()
{
	int i = 10;

	Anything intAny = i;
	assertEqual(i, intAny.AsLong(0));

	Anything b = intAny;

	i = -10;
	intAny = i;

	assertEqual(i, intAny.AsLong(0));
	assertEqual(b.AsLong(0), 10L); // still 10
}

void AnythingTest::ifaObjectOperatorAssign()
{
	DummyIFAObj o("test");
	Anything objAny = &o;

	assertEqual((long)(&o), (long) objAny.AsIFAObject(0));
	assertEqual((long)(&o), (long) objAny.AsLong(0));
	assertEqual((long)0.0, (long) objAny.AsDouble(0.0));
	assertEqual("IFAObject", objAny.AsCharPtr(""));
	assertEqual((long)(false), (long) objAny.AsBool(false));
}

void AnythingTest::roConversion()
{
	boolROConversion();
	intROConversion();
	ifaObjectROConversion();
}

void AnythingTest::boolROConversion()
{
	// test bool operator
	bool b = true;

	Anything boolAny = b;
	ROAnything roBoolAny(boolAny);
	assertEqual((long)(b), (long) roBoolAny.AsBool(false));

	b = false;
	boolAny = b;
	roBoolAny = boolAny;
	assertEqual((long)(b), (long) roBoolAny.AsBool(true));

	boolAny = 1;
	roBoolAny = boolAny;
	assertEqual((long)(true), (long) roBoolAny.AsBool(false));

	boolAny = 0;
	roBoolAny = boolAny;
	assertEqual((long)(false), (long) roBoolAny.AsBool(false));

	// some quirky cases
	long c = -1;
	long c1 = 1;
	boolAny = ((IFAObject *) c);
	roBoolAny = boolAny;
	assertEqual((long)(false), (long) roBoolAny.AsBool(false));

	boolAny = ((IFAObject *) c1);
	roBoolAny = boolAny;
	assertEqual((long)(false), (long) roBoolAny.AsBool(false));
}

void AnythingTest::intROConversion()
{
	int i = 10;

	Anything intAny = i;
	ROAnything roIntAny(intAny);
	assertEqual(i, roIntAny.AsLong(0));

	i = -10;
	intAny = i;
	roIntAny = intAny;

	assertEqual(i, roIntAny.AsLong(0));
}

void AnythingTest::ifaObjectROConversion()
{
	DummyIFAObj o("test");
	Anything objAny = &o;
	ROAnything roObjAny(objAny);

	assertEqual((long)(&o), (long) roObjAny.AsIFAObject(0));
	assertEqual((long)(&o), (long) roObjAny.AsLong(0));
	assertEqual((long)0.0, (long) roObjAny.AsDouble(0.0));
	assertEqual("IFAObject", roObjAny.AsCharPtr(""));
	assertEqual((long)(false), (long) roObjAny.AsBool(false));
}

void AnythingTest::BinaryBufOutput()
{
	Anything testAny;
	testAny["T1"] = Anything((void *) "1234567890", 10);
	String testString;
	OStringStream os(&testString);

	os << testAny << std::flush;

	assertEqual( "{\n  /T1 [10;1234567890]\n}", testString);
	assertEqual(25, testString.Length());
}

void AnythingTest::String2LongConversion()
{
	Anything a("1212");
	long l = a.AsLong(-1);
	assertEqual(1212, l);
}

void AnythingTest::AsCharPtrBufLen()
{
	//--- AnyArrayImpl
	Anything anyArray=Anything(Anything::MetaMarker());

	long arraylen;
	anyArray.AsCharPtr("", arraylen);
	assertEqual(12, arraylen); // strlen("AnyArrayImpl")

	//--- AnyBinaryBufImpl
	char test[10];
	memset(test, '\0', 10);
	Anything anyTest((void *) test, 10);
	long buflen;
	anyTest.AsCharPtr("", buflen);
	assertEqual(10, buflen);

	//--- AnyDoubleImpl
	Anything anydouble(2.01);
	long doublelen;
	anydouble.AsCharPtr("", doublelen);
	t_assert( 3 <= doublelen );

	//--- AnyLongImpl
	Anything anylong(100);
	long longlen;
	anylong.AsCharPtr("", longlen);
	assertEqual(3, longlen); // strlen("100")

	//--- AnyObjectImpl
	IFAObject *o = 0;
	Anything anyobject(o);
	long objectlen;
	anyobject.AsCharPtr("", objectlen);
	assertEqual(9, objectlen); // strlen("IFAObject")

	//--- AnyStringImpl
	Anything anystring("abc");
	long stringlen;
	anystring.AsCharPtr("", stringlen);
	assertEqual(3, stringlen); // strlen("abc")
}

void AnythingTest::RefCount()
{
	Anything a;
	Anything b;

	// empty anything can't have shared impls therefore the refcount is always 0
	assertCompare( a.RefCount(), equal_to, 0L );
	assertCompare( b.RefCount(), equal_to, 0L );

	a = b;
	assertCompare( a.RefCount(), equal_to, 0L );
	assertCompare( b.RefCount(), equal_to, 0L );

	// assign a string -> implicitly create an impl
	a = "test the ref";
	assertCompare( a.RefCount(), equal_to, 1L );
	assertCompare( b.RefCount(), equal_to, 0L );

	b = a;
	assertCompare( a.RefCount(), equal_to, 2L );
	assertCompare( b.RefCount(), equal_to, 2L );

	a = Anything();
	assertCompare( a.RefCount(), equal_to, 0L );
	assertCompare( b.RefCount(), equal_to, 1L );

	b = Anything();
	assertCompare( a.RefCount(), equal_to, 0L );
	assertCompare( b.RefCount(), equal_to, 0L );
}

void AnythingTest::SlotCopierTest()
{
	StartTrace(AnythingTest.SlotCopierTest);
	// Set up
	for (long l = 0; l < fQuery.GetSize(); l++) {
		ROAnything config = fQuery[l];
		ROAnything copyList = config["CopyList"];
		Anything source = config["Source"].DeepClone();
		Anything result;

		SlotCopier::Operate(source, result, copyList,
							config["Delim"].AsCharPtr(".")[0L],
							config["IndexDelim"].AsCharPtr(":")[0L]);

		ROAnything expectedStore(config["Result"]);
		assertAnyEqualm(expectedStore, result, TString(name()) << "." << fQuery.SlotName(l));
	}
}

void AnythingTest::SlotFinderTest()
{
	StartTrace(AnythingTest.SlotFinderTest);
	// Set up
	for (long l = 0; l < fQuery.GetSize(); l++) {
		ROAnything config = fQuery[l];
		Anything source = fConfig["SlotFinderStore"];
		Anything foundResult;
		SlotFinder::Operate(source, foundResult, config);
		ROAnything expectedStore(config["Result"]);
		assertAnyEqualm(expectedStore, foundResult, TString(name()) << "." << fQuery.SlotName(l));
	}
}

void AnythingTest::SlotPutterTest()
{
	StartTrace(AnythingTest.SlotPutterTest);
	// Set up
	for (long l = 0; l < fQuery.GetSize(); l++) {
		TraceAny(fQuery[l], String("testconfig of ") << fQuery.SlotName(l));
		Anything config = fQuery[l]["Destination"];
		Anything toStore = fQuery[l]["ToStore"];
		Anything tmpStore;
		if (fQuery[l].IsDefined("Store")) {
			fQuery[l].LookupPath(tmpStore, "Store");
		}
		SlotPutter::Operate(toStore, tmpStore, config);
		Anything expectedStore(fQuery[l]["Result"]);
		assertAnyEqualm(expectedStore, tmpStore, TString(name()) << "." << fQuery.SlotName(l));
	}
}

void AnythingTest::SlotPutterAppendTest()
{
	StartTrace(AnythingTest.SlotPutterAppendTest);
	// Set up
	Anything config = fQuery["Destination"];
	Anything rStore = fQuery["receiver"];
	Anything toStore = fQuery["firstItem"];

	TraceAny(toStore, "toStore");
	TraceAny(rStore, "firstItem");
	SlotPutter::Operate(toStore, rStore, config);

	toStore = fQuery["secondItem"];

	TraceAny(toStore, "toStore second");
	SlotPutter::Operate(toStore, rStore, config);

	TraceAny(rStore, "rStore after");

	Anything expectedStore(fConfig["Results"][name()]);
	assertAnyEqual(expectedStore, rStore);
}

void AnythingTest::AnythingLeafIteratorTest()
{
	StartTrace(AnythingTest.AnythingLeafIteratorTest);
	ROAnything expectedStore(fConfig["Results"][name()]);
	{
		// Set up
		Anything iterateMe = fQuery["IterateThis"];

		Anything foundResult;
		AnyExtensions::LeafIterator<Anything> iter(iterateMe);

		Anything akt;
		while (iter.Next(akt)) {
			foundResult.Append(akt);
		}
		assertAnyEqual(expectedStore, foundResult);
	}
	{
		// Set up
		Anything foundResult;
		AnyExtensions::LeafIterator<ROAnything> iter(fQuery["IterateThis"]);

		ROAnything akt;
		while (iter.Next(akt)) {
			foundResult.Append(akt.DeepClone());
		}
		assertAnyEqual(expectedStore, foundResult);
	}
	{
		// Set up
		ROAnything iterateMe = fQuery["IterateThis"];

		Anything foundResult;
		AnyExtensions::LeafIterator<ROAnything> iter(iterateMe);

		ROAnything akt;
		while (iter.Next(akt)) {
			foundResult.Append(akt.DeepClone());
		}

		assertAnyEqual(expectedStore, foundResult);
	}
}

void AnythingTest::SlotnameSorterTest()
{
	StartTrace(AnythingTest.SlotnameSorterTest);
	ROAnything roaTestConfig;
	if (t_assert(((ROAnything)fConfig).LookupPath(roaTestConfig, "SlotnameSorterTest"))) {
		ROAnything cConfig(roaTestConfig[0L]);
		for (long lIdx = 0; lIdx < roaTestConfig.GetSize(); lIdx++, cConfig
			 = roaTestConfig[lIdx]) {
			TraceAny(cConfig, "the config");
			Anything sorted;
			SlotnameSorter::EMode mode(
				cConfig["Mode"].AsString() == "asc" ? SlotnameSorter::asc
				: SlotnameSorter::desc);
			sorted = cConfig["TestArray"].DeepClone();
			SlotnameSorter::Sort(sorted, mode);
			assertAnyEqual( cConfig["ExpectedResult"], sorted);
		}
	}
}

void AnythingTest::SlotCleanerTest()
{
	StartTrace(AnythingTest.SlotCleanerTest);
	// Set up
	for (long l = 0; l < fQuery.GetSize(); l++) {
		Anything config = fQuery[l];
		Anything removeFromThis = config["RemoveFromThis"];
		Anything removePath = config["RemovePath"];

		SlotCleaner::Operate(removeFromThis, removePath);

		Anything expectedStore(config["Result"]);
		assertAnyEqualm(expectedStore, removeFromThis, TString(name()) << "." << fQuery.SlotName(l));
	}
}

/*================================================================================================================*/
/*                           M I S C E L L A N E O U S     T E S T S   end                                        */
/*================================================================================================================*/
