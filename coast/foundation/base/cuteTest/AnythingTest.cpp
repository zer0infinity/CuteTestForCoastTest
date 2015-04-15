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

#include "AnythingTest.h"
#include "../anythingoptional/AnyIterators.h"
#include "IFAObject.h"

using namespace coast;

AnythingTest::AnythingTest() {
	StartTrace(AnythingTest.setUp);
	if (!system::LoadConfigFile(fConfig, "AnythingTest", "any")) {
		ASSERT_EQUAL( "'read AnythingTest.any'", "'could not read AnythingTest.any'" );
	}
	fQuery = fConfig["Queries"];
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

void AnythingTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(AnythingTest, TypeTest));
	s.push_back(CUTE_SMEMFUN(AnythingTest, SuccessiveAssignments));
	s.push_back(CUTE_SMEMFUN(AnythingTest, BinaryBufOutput));
	s.push_back(CUTE_SMEMFUN(AnythingTest, roConversion));
	s.push_back(CUTE_SMEMFUN(AnythingTest, operatorAssignemnt));
	s.push_back(CUTE_SMEMFUN(AnythingTest, appendTest));
	s.push_back(CUTE_SMEMFUN(AnythingTest, String2LongConversion));
	s.push_back(CUTE_SMEMFUN(AnythingTest, AsCharPtrBufLen));
	s.push_back(CUTE_SMEMFUN(AnythingTest, RefCount));

	s.push_back(CUTE_SMEMFUN(AnythingTest, AnythingLeafIteratorTest));
	s.push_back(CUTE_SMEMFUN(AnythingTest, SlotFinderTest));
	s.push_back(CUTE_SMEMFUN(AnythingTest, SlotCopierTest));
	s.push_back(CUTE_SMEMFUN(AnythingTest, SlotPutterTest));
	s.push_back(CUTE_SMEMFUN(AnythingTest, SlotPutterAppendTest));
	s.push_back(CUTE_SMEMFUN(AnythingTest, SlotnameSorterTest));
	s.push_back(CUTE_SMEMFUN(AnythingTest, SlotCleanerTest));
}

/*======================================================================================================================*/
/*  C L A S S M E T H O D S  (helpmethods)       Ende                                                                   */
/*======================================================================================================================*/

void AnythingTest::TypeTest() {
	Anything TypeChange;
	Anything ArrayTypeTest;
	Anything KeyTypeTest;
	Anything AnyLong(5L);
	Anything AnyVoidBuff((void *) "Bytebuffer", 10);
	Anything SubAnything;
	String MyString("Another String");

	// Simple Types
	ASSERT( TypeChange.GetType() == AnyNullType );

	TypeChange = 12L;
	ASSERT( TypeChange.GetType() == AnyLongType );

	TypeChange = (char *) "Ein String";
	ASSERT( TypeChange.GetType() == AnyCharPtrType );

	TypeChange = (long) 34;
	ASSERT( TypeChange.GetType() == AnyLongType );

	TypeChange = (double) 300.44;
	ASSERT( TypeChange.GetType() == AnyDoubleType );

	TypeChange = (long) 56;
	ASSERT( TypeChange.GetType() == AnyLongType );

	TypeChange = MyString;
	ASSERT( TypeChange.GetType() == AnyCharPtrType );

	TypeChange = (float) 1.37;
	ASSERT( TypeChange.GetType() == AnyDoubleType );

	TypeChange = AnyLong;
	ASSERT( TypeChange.GetType() == AnyLongType );

	// Array
	ArrayTypeTest[""] = ("Noch ein String");
	ASSERT( ArrayTypeTest.GetType() == AnyArrayType );
	ASSERT( ArrayTypeTest[0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[""] = (12L);
	ASSERT( ArrayTypeTest.GetType() == AnyArrayType );
	ASSERT( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	ASSERT( ArrayTypeTest[ 1].GetType() == AnyLongType );

	ArrayTypeTest[""] = ((long) 34);
	ASSERT( ArrayTypeTest.GetType() == AnyArrayType );
	ASSERT( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	ASSERT( ArrayTypeTest[ 1].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 2].GetType() == AnyLongType );

	ArrayTypeTest[""] = ((long) 56);
	ASSERT( ArrayTypeTest.GetType() == AnyArrayType );
	ASSERT( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	ASSERT( ArrayTypeTest[ 1].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 2].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 3].GetType() == AnyLongType );

	ArrayTypeTest[""] = ((float) 98.765);
	ASSERT( ArrayTypeTest.GetType() == AnyArrayType );
	ASSERT( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	ASSERT( ArrayTypeTest[ 1].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 2].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 3].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 4].GetType() == AnyDoubleType );

	ArrayTypeTest[""] = ((double) 12.345);
	ASSERT( ArrayTypeTest.GetType() == AnyArrayType );
	ASSERT( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	ASSERT( ArrayTypeTest[ 1].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 2].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 3].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 4].GetType() == AnyDoubleType );
	ASSERT( ArrayTypeTest[ 5].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = 12L;
	ASSERT( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = (char *) "Ein String";
	ASSERT( ArrayTypeTest[0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[0L] = 34L;
	ASSERT( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = (double) 300.44;
	ASSERT( ArrayTypeTest[0L].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = (long) 56;
	ASSERT( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = (float) 1.37;
	ASSERT( ArrayTypeTest[0L].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = AnyLong;
	ASSERT( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = MyString;
	ASSERT( ArrayTypeTest[0L].GetType() == AnyCharPtrType );

	SubAnything = ArrayTypeTest;
	ArrayTypeTest.Append(SubAnything); // this will lead to a leak!
	ASSERT( ArrayTypeTest.GetType() == AnyArrayType );
	ASSERT( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	ASSERT( ArrayTypeTest[ 1].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 2].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 3].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[ 4].GetType() == AnyDoubleType );
	ASSERT( ArrayTypeTest[ 5].GetType() == AnyDoubleType );
	ASSERT( ArrayTypeTest[ 6].GetType() == AnyArrayType );
	ASSERT( ArrayTypeTest[6][0L].GetType() == AnyCharPtrType );
	ASSERT( ArrayTypeTest[6][ 1].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[6][ 2].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[6][ 3].GetType() == AnyLongType );
	ASSERT( ArrayTypeTest[6][ 4].GetType() == AnyDoubleType );
	ASSERT( ArrayTypeTest[6][ 5].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = 12L;
	ASSERT( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest[6][0L] = (char *) "Ein String";
	ASSERT( ArrayTypeTest[6][0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[6][0L] = (long) 34;
	ASSERT( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest[6][0L] = (double) 300.44;
	ASSERT( ArrayTypeTest[6][0L].GetType() == AnyDoubleType );

	ArrayTypeTest[6][0L] = (long) 56;
	ASSERT( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest[6][0L] = MyString;
	ASSERT( ArrayTypeTest[6][0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[6][0L] = (float) 1.37;
	ASSERT( ArrayTypeTest[6][0L].GetType() == AnyDoubleType );

	ArrayTypeTest[6][0L] = AnyLong;
	ASSERT( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest.Remove(6L); // break circular dependency to avoid leak

	// Array
	KeyTypeTest["0"] = "Noch ein String";
	ASSERT( KeyTypeTest.GetType() == AnyArrayType );
	ASSERT( KeyTypeTest["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["1"] = (12L);
	ASSERT( KeyTypeTest.GetType() == AnyArrayType );
	ASSERT( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	ASSERT( KeyTypeTest["1"].GetType() == AnyLongType );

	KeyTypeTest["2"] = ((long) 34);
	ASSERT( KeyTypeTest.GetType() == AnyArrayType );
	ASSERT( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	ASSERT( KeyTypeTest["1"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["2"].GetType() == AnyLongType );

	KeyTypeTest["3"] = ((long) 56);
	ASSERT( KeyTypeTest.GetType() == AnyArrayType );
	ASSERT( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	ASSERT( KeyTypeTest["1"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["2"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["3"].GetType() == AnyLongType );

	KeyTypeTest["4"] = ((float) 98.765);
	ASSERT( KeyTypeTest.GetType() == AnyArrayType );
	ASSERT( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	ASSERT( KeyTypeTest["1"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["2"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["3"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["4"].GetType() == AnyDoubleType );

	KeyTypeTest["5"] = ((double) 12.345);
	ASSERT( KeyTypeTest.GetType() == AnyArrayType );
	ASSERT( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	ASSERT( KeyTypeTest["1"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["2"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["3"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["4"].GetType() == AnyDoubleType );
	ASSERT( KeyTypeTest["5"].GetType() == AnyDoubleType );

	KeyTypeTest["0"] = 12L;
	ASSERT( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = (char *) "Ein String";
	ASSERT( KeyTypeTest["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["0"] = (long) 34;
	ASSERT( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = (double) 300.44;
	ASSERT( KeyTypeTest["0"].GetType() == AnyDoubleType );

	KeyTypeTest["0"] = (long) 56;
	ASSERT( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = (float) 1.37;
	ASSERT( KeyTypeTest["0"].GetType() == AnyDoubleType );

	KeyTypeTest["0"] = AnyLong;
	ASSERT( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = MyString;
	ASSERT( KeyTypeTest["0"].GetType() == AnyCharPtrType );

	SubAnything = KeyTypeTest;
	KeyTypeTest["6"] = (SubAnything);
	ASSERT( KeyTypeTest.GetType() == AnyArrayType );
	ASSERT( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	ASSERT( KeyTypeTest["1"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["2"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["3"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["4"].GetType() == AnyDoubleType );
	ASSERT( KeyTypeTest["5"].GetType() == AnyDoubleType );
	ASSERT( KeyTypeTest["6"].GetType() == AnyArrayType );
	ASSERT( KeyTypeTest["6"]["0"].GetType() == AnyCharPtrType );
	ASSERT( KeyTypeTest["6"]["1"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["6"]["2"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["6"]["3"].GetType() == AnyLongType );
	ASSERT( KeyTypeTest["6"]["4"].GetType() == AnyDoubleType );
	ASSERT( KeyTypeTest["6"]["5"].GetType() == AnyDoubleType );

	KeyTypeTest["6"]["0"] = 12L;
	ASSERT( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest["6"]["0"] = (char *) "Ein String";
	ASSERT( KeyTypeTest["6"]["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["6"]["0"] = (long) 34;
	ASSERT( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest["6"]["0"] = (double) 300.44;
	ASSERT( KeyTypeTest["6"]["0"].GetType() == AnyDoubleType );

	KeyTypeTest["6"]["0"] = (long) 56;
	ASSERT( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest["6"]["0"] = MyString;
	ASSERT( KeyTypeTest["6"]["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["6"]["0"] = (float) 1.37;
	ASSERT( KeyTypeTest["6"]["0"].GetType() == AnyDoubleType );

	KeyTypeTest["6"]["0"] = AnyLong;
	ASSERT( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest.Remove("6"); // break circular dependency to avoid leak
}

void AnythingTest::SuccessiveAssignments()
// some random assignments with a strange outcome
{
	Anything a = "Test";
	ASSERT(!a.IsDefined("Test"));
	ASSERT_EQUAL("Test", a.AsCharPtr("nothing"));

	if (a.IsDefined("Test")) {
		a["Test"] = "Was ist hier los ?";
	}

	Anything b = "Test";
	ASSERT_EQUAL("Test", b.AsCharPtr("nothing"));
	Anything c = b["Test"];
	ASSERT( b.IsDefined("Test") );
	ASSERT( c.IsNull() );
} // SuccessiveAssignments

void AnythingTest::operatorAssignemnt() {
	boolOperatorAssign();
	intOperatorAssign();
	ifaObjectOperatorAssign();
}

void AnythingTest::appendTest() {
	Anything sub;
	sub["X"] = "x";
	sub["Y"] = "y";

	Anything a;
	a.Append(sub);
	Anything expected;
	expected[0L] = sub;
	ASSERT_EQUAL(expected, a);

	a.Append(sub);
	expected[1L] = sub;
	ASSERT_EQUAL(expected, a);

	Anything b = "Test"; // AnyStringImpl
	b.Append(sub); // -> AnyArrayImpl
	expected[0L] = "Test";
	ASSERT_EQUAL(expected, b);

	b["NewKey"].Append(sub);
	expected["NewKey"][0L] = sub;
	ASSERT_EQUAL(expected, b);
}

void AnythingTest::boolOperatorAssign() {
	// test bool operator
	bool b = true;

	Anything boolAny = b;
	ASSERT_EQUAL((long)(b), (long) boolAny.AsBool(false));

	b = false;
	boolAny = b;
	ASSERT_EQUAL((long)(b), (long) boolAny.AsBool(true));

	boolAny = 1;
	ASSERT_EQUAL((long)(true), (long) boolAny.AsBool(false));

	boolAny = 0;
	ASSERT_EQUAL((long)(false), (long) boolAny.AsBool(false));

	// some quirky cases
	long c = -1;
	long c1 = 1;
	boolAny = ((IFAObject *) c);
	ASSERT_EQUAL((long)(false), (long) boolAny.AsBool(false));

	boolAny = ((IFAObject *) c1);
	ASSERT_EQUAL((long)(false), (long) boolAny.AsBool(false));
}

void AnythingTest::intOperatorAssign() {
	int i = 10;

	Anything intAny = i;
	ASSERT_EQUAL(i, intAny.AsLong(0));

	Anything b = intAny;

	i = -10;
	intAny = i;

	ASSERT_EQUAL(i, intAny.AsLong(0));
	ASSERT_EQUAL(b.AsLong(0), 10L); // still 10
}

void AnythingTest::ifaObjectOperatorAssign() {
	DummyIFAObj o("test");
	Anything objAny = &o;

	ASSERT_EQUAL((long)(&o), (long) objAny.AsIFAObject(0));
	ASSERT_EQUAL((long)(&o), (long) objAny.AsLong(0));
	ASSERT_EQUAL((long)0.0, (long) objAny.AsDouble(0.0));
	ASSERT_EQUAL("IFAObject", objAny.AsCharPtr(""));
	ASSERT_EQUAL((long)(false), (long) objAny.AsBool(false));
}

void AnythingTest::roConversion() {
	boolROConversion();
	intROConversion();
	ifaObjectROConversion();
}

void AnythingTest::boolROConversion() {
	// test bool operator
	bool b = true;

	Anything boolAny = b;
	ROAnything roBoolAny(boolAny);
	ASSERT_EQUAL((long)(b), (long) roBoolAny.AsBool(false));

	b = false;
	boolAny = b;
	roBoolAny = boolAny;
	ASSERT_EQUAL((long)(b), (long) roBoolAny.AsBool(true));

	boolAny = 1;
	roBoolAny = boolAny;
	ASSERT_EQUAL((long)(true), (long) roBoolAny.AsBool(false));

	boolAny = 0;
	roBoolAny = boolAny;
	ASSERT_EQUAL((long)(false), (long) roBoolAny.AsBool(false));

	// some quirky cases
	long c = -1;
	long c1 = 1;
	boolAny = ((IFAObject *) c);
	roBoolAny = boolAny;
	ASSERT_EQUAL((long)(false), (long) roBoolAny.AsBool(false));

	boolAny = ((IFAObject *) c1);
	roBoolAny = boolAny;
	ASSERT_EQUAL((long)(false), (long) roBoolAny.AsBool(false));
}

void AnythingTest::intROConversion() {
	int i = 10;

	Anything intAny = i;
	ROAnything roIntAny(intAny);
	ASSERT_EQUAL(i, roIntAny.AsLong(0));

	i = -10;
	intAny = i;
	roIntAny = intAny;

	ASSERT_EQUAL(i, roIntAny.AsLong(0));
}

void AnythingTest::ifaObjectROConversion() {
	DummyIFAObj o("test");
	Anything objAny = &o;
	ROAnything roObjAny(objAny);

	ASSERT_EQUAL((long)(&o), (long) roObjAny.AsIFAObject(0));
	ASSERT_EQUAL((long)(&o), (long) roObjAny.AsLong(0));
	ASSERT_EQUAL((long)0.0, (long) roObjAny.AsDouble(0.0));
	ASSERT_EQUAL("IFAObject", roObjAny.AsCharPtr(""));
	ASSERT_EQUAL((long)(false), (long) roObjAny.AsBool(false));
}

void AnythingTest::BinaryBufOutput() {
	Anything testAny;
	testAny["T1"] = Anything((void *) "1234567890", 10);
	String tesString;
	OStringStream os(&tesString);

	os << testAny << std::flush;

	ASSERT_EQUAL( "{\n  /T1 [10;1234567890]\n}", tesString);
	ASSERT_EQUAL(25, tesString.Length());
}

void AnythingTest::String2LongConversion() {
	Anything a("1212");
	long l = a.AsLong(-1);
	ASSERT_EQUAL(1212, l);
}

void AnythingTest::AsCharPtrBufLen() {
	//--- AnyArrayImpl
	Anything anyArray = Anything(Anything::ArrayMarker());

	long arraylen;
	anyArray.AsCharPtr("", arraylen);
	ASSERT_EQUAL(12, arraylen); // strlen("AnyArrayImpl")

	//--- AnyBinaryBufImpl
	char test[10];
	memset(test, '\0', 10);
	Anything anyTest((void *) test, 10);
	long buflen;
	anyTest.AsCharPtr("", buflen);
	ASSERT_EQUAL(10, buflen);

	//--- AnyDoubleImpl
	Anything anydouble(2.01);
	long doublelen;
	anydouble.AsCharPtr("", doublelen);
	ASSERT( 3 <= doublelen );

	//--- AnyLongImpl
	Anything anylong(100);
	long longlen;
	anylong.AsCharPtr("", longlen);
	ASSERT_EQUAL(3, longlen); // strlen("100")

	//--- AnyObjectImpl
	IFAObject *o = 0;
	Anything anyobject(o);
	long objectlen;
	anyobject.AsCharPtr("", objectlen);
	ASSERT_EQUAL(9, objectlen); // strlen("IFAObject")

	//--- AnyStringImpl
	Anything anystring("abc");
	long stringlen;
	anystring.AsCharPtr("", stringlen);
	ASSERT_EQUAL(3, stringlen); // strlen("abc")
}

void AnythingTest::RefCount() {
	Anything a;
	Anything b;

	// empty anything can't have shared impls therefore the refcount is always 0
	ASSERT_EQUAL( a.RefCount(), 0L );
	ASSERT_EQUAL( b.RefCount(), 0L );

	a = b;
	ASSERT_EQUAL( a.RefCount(), 0L );
	ASSERT_EQUAL( b.RefCount(), 0L );

	// assign a string -> implicitly create an impl
	a = "test the ref";
	ASSERT_EQUAL( a.RefCount(), 1L );
	ASSERT_EQUAL( b.RefCount(), 0L );

	b = a;
	ASSERT_EQUAL( a.RefCount(), 2L );
	ASSERT_EQUAL( b.RefCount(), 2L );

	a = Anything();
	ASSERT_EQUAL( a.RefCount(), 0L );
	ASSERT_EQUAL( b.RefCount(), 1L );

	b = Anything();
	ASSERT_EQUAL( a.RefCount(), 0L );
	ASSERT_EQUAL( b.RefCount(), 0L );
}

void AnythingTest::SlotCopierTest() {
	StartTrace(AnythingTest.SlotCopierTest);
	// Set up
	for (long l = 0; l < fQuery.GetSize(); l++) {
		ROAnything config = fQuery[l];
		ROAnything copyList = config["CopyList"];
		Anything source = config["Source"].DeepClone();
		Anything result;

		SlotCopier::Operate(source, result, copyList, config["Delim"].AsCharPtr(".")[0L], config["IndexDelim"].AsCharPtr(":")[0L]);

		ROAnything expectedStore(config["Result"]);
		String msg;
		msg << "." << fQuery.SlotName(l);
		ASSERT_EQUALM(msg.cstr(), expectedStore, result);
	}
}

void AnythingTest::SlotFinderTest() {
	StartTrace(AnythingTest.SlotFinderTest);
	// Set up
	for (long l = 0; l < fQuery.GetSize(); l++) {
		ROAnything config = fQuery[l];
		Anything source = fConfig["SlotFinderStore"];
		Anything foundResult;
		SlotFinder::Operate(source, foundResult, config);
		ROAnything expectedStore(config["Result"]);
		String msg;
		msg << "." << fQuery.SlotName(l);
		ASSERT_EQUALM(msg.cstr(), expectedStore, foundResult);
	}
}

void AnythingTest::SlotPutterTest() {
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
		String msg;
		msg << "." << fQuery.SlotName(l);
		ASSERT_EQUALM(msg.cstr(), expectedStore, tmpStore);
	}
}

void AnythingTest::SlotPutterAppendTest() {
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

	Anything expectedStore(fConfig["Results"]);
	ASSERT_EQUAL(expectedStore, rStore);
}

void AnythingTest::AnythingLeafIteratorTest() {
	StartTrace(AnythingTest.AnythingLeafIteratorTest);
	ROAnything expectedStore(fConfig["Results"]);
	{
		// Set up
		Anything iterateMe = fQuery["IterateThis"];

		Anything foundResult;
		AnyExtensions::LeafIterator<Anything> iter(iterateMe);

		Anything akt;
		while (iter.Next(akt)) {
			foundResult.Append(akt);
		}
		ASSERT_EQUAL(expectedStore, foundResult);
	}
	{
		// Set up
		Anything foundResult;
		AnyExtensions::LeafIterator<ROAnything> iter(fQuery["IterateThis"]);

		ROAnything akt;
		while (iter.Next(akt)) {
			foundResult.Append(akt.DeepClone());
		}
		ASSERT_EQUAL(expectedStore, foundResult);
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

		ASSERT_EQUAL(expectedStore, foundResult);
	}
}

void AnythingTest::SlotnameSorterTest() {
	StartTrace(AnythingTest.SlotnameSorterTest);
	ROAnything roaTestConfig;
	ASSERT(((ROAnything)fConfig).LookupPath(roaTestConfig, "SlotnameSorterTest"));
		ROAnything cConfig(roaTestConfig[0L]);
		for (long lIdx = 0; lIdx < roaTestConfig.GetSize(); lIdx++, cConfig = roaTestConfig[lIdx]) {
			TraceAny(cConfig, "the config");
			Anything sorted;
			SlotnameSorter::EMode mode(cConfig["Mode"].AsString() == "asc" ? SlotnameSorter::asc : SlotnameSorter::desc);
			sorted = cConfig["TestArray"].DeepClone();
			SlotnameSorter::Sort(sorted, mode);
			ASSERT_EQUAL( cConfig["ExpectedResult"], sorted);

	}
}

void AnythingTest::SlotCleanerTest() {
	StartTrace(AnythingTest.SlotCleanerTest);
	// Set up
	for (long l = 0; l < fQuery.GetSize(); l++) {
		Anything config = fQuery[l];
		Anything removeFromThis = config["RemoveFromThis"];
		Anything removePath = config["RemovePath"];

		SlotCleaner::Operate(removeFromThis, removePath);

		Anything expectedStore(config["Result"]);
		String msg;
		msg << "." << fQuery.SlotName(l);
		ASSERT_EQUALM(msg.cstr(), expectedStore, removeFromThis);
	}
}

/*================================================================================================================*/
/*                           M I S C E L L A N E O U S     T E S T S   end                                        */
/*================================================================================================================*/
