/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnythingConstructorsTest.h"
#include "TestSuite.h"
#include "IFAObject.h"
#include "Tracer.h"

void AnythingConstructorsTest::setUp() {
	StartTrace(AnythingConstructorsTest.setUp);
	fString = "A String";
	fLong = 5L;
	fBool = true;
	fDouble = 7.125; // there is an exact binary representation
	fDouble2 = 8.1; // there is no exact binary representation
	fFloat = -24.490123456789;
	fNull = Anything();
}

Test *AnythingConstructorsTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnythingConstructorsTest, DefaultConstrTest);
	ADD_CASE(testSuite, AnythingConstructorsTest, IntConstrTest);
	ADD_CASE(testSuite, AnythingConstructorsTest, LongConstrTest);
	ADD_CASE(testSuite, AnythingConstructorsTest, FloatConstrTest);
	ADD_CASE(testSuite, AnythingConstructorsTest, DoubleConstr0Test);
	ADD_CASE(testSuite, AnythingConstructorsTest, DoubleConstr1Test);
	ADD_CASE(testSuite, AnythingConstructorsTest, CharStarConstrTest);
	ADD_CASE(testSuite, AnythingConstructorsTest, CharStarLongConstr0Test);
	ADD_CASE(testSuite, AnythingConstructorsTest, CharStarLongConstr1Test);
	ADD_CASE(testSuite, AnythingConstructorsTest, CharStarLongConstr2Test);
	ADD_CASE(testSuite, AnythingConstructorsTest, CharStarLongConstr3Test);
	ADD_CASE(testSuite, AnythingConstructorsTest, StringConstrTest);
	ADD_CASE(testSuite, AnythingConstructorsTest, VoidStarLenConstrTest);
	ADD_CASE(testSuite, AnythingConstructorsTest, EmptyVoidStarLenConstrTest);
	ADD_CASE(testSuite, AnythingConstructorsTest, IFAObjectStarConstrTest);
	ADD_CASE(testSuite, AnythingConstructorsTest, AnythingConstrTest);
	return testSuite;
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

void AnythingConstructorsTest::DefaultConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	Anything anyHlp;

	t_assert( fNull.GetType() == AnyNullType );
	assertCompare( fNull.IsNull(), equal_to, true );
	assertCompare( fNull.IsNull(), equal_to, true );
	t_assert( fNull.IsNull() == bool(1) );
	t_assert( fNull.GetSize() == bool(0) );
	t_assert( fNull == anyHlp );
	t_assert( !(fNull != anyHlp) );
	t_assert( fNull.IsEqual(anyHlp) );

	t_assert( String("") == fNull.AsCharPtr() );
	t_assert( String("Default") == fNull.AsCharPtr("Default") );
	t_assert( fNull.AsCharPtr(0) == NULL );
	assertCompare( fNull.AsLong(), equal_to, 0L );
	assertCompare( fNull.AsLong(1234), equal_to, 1234L );
	assertCompare( fNull.AsBool(), equal_to, false );
	assertCompare( fNull.AsBool(true), equal_to, true );
	t_assert( fNull.AsBool() == 0 );
	t_assert( fNull.AsDouble() == 0 );
	assertCompare( fNull.AsDouble(2.3), equal_to, 2.3 );
	t_assert( fNull.AsIFAObject() == NULL );

	DummyIFAObj testObj("testObj");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( fNull.AsIFAObject(&testObj) == &testObj );

	assertCompare( (fNull.AsString()).Length(), equal_to, 0L );
	t_assert( (fNull.AsString()).Capacity() >= (fNull.AsString()).Length() );
	t_assert( fNull.AsString() == "" );
	t_assert( fNull.AsString("Default") == "Default" );

	t_assert( fNull.Contains("testSlot") == false );
	assertCompare( fNull.FindValue("testSlot"), less, 0L );

	// Does it survive?
	fNull.Remove(-1L);
	fNull.Remove(0L);
	fNull.Remove(1L);
	assertCompare( fNull.FindIndex("testSlots"), less, 0L );
	t_assert( fNull.IsDefined("testSlots") == false );
	t_assert( fNull.SlotName(0) == NULL );
	//t_assert( fNull.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	t_assert( fNull.At(0L) == NULL );
	t_assert( fNull.At(1L) == NULL );
	//t_assert( fNull[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	t_assert( fNull[0L] == NULL );
	t_assert( fNull[1L] == NULL );
}

void AnythingConstructorsTest::IntConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	Anything anyHlp = fBool;

	t_assert( fBool.GetType() == AnyLongType );
	assertCompare( fBool.IsNull(), equal_to, false );
	assertCompare( fBool.IsNull(), equal_to, false );
	t_assert( fBool.IsNull() == 0 );
	assertCompare( fBool.GetSize(), equal_to, 1L );
	t_assert( fBool == anyHlp );
	t_assert( !(fBool != anyHlp) );
	t_assert( fBool.IsEqual(anyHlp) );

	assertCompare( fBool.AsBool(), equal_to, true );
	assertCompare( fBool.AsBool(false), equal_to, true );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( fBool.AsIFAObject(&testObj) == &testObj );
	t_assert( fBool.AsIFAObject(&testObj) != &testObjDummy );
	t_assert( fBool.AsIFAObject() == NULL );

	assertCompare( (fBool.AsString()).Length(), equal_to, 1L );
	t_assert( (fBool.AsString()).Capacity() >= (fBool.AsString()).Length() );

	assertCompare( fBool.Contains("testSlot"), equal_to, false );
	assertCompare( fBool.FindValue("testSlot"), less, 0L );

	// Does it survive?
	fBool.Remove(-1L);
	fBool.Remove(0L);
	fBool.Remove(1L);
	assertCompare( fBool.FindIndex("testSlots"), less, 0L );
	assertCompare( fBool.IsDefined("testSlots"), equal_to, false );
	t_assert( fBool.SlotName(0) == NULL );
	assertCompare( (fBool.At(0L)).AsBool(false), equal_to, false );
	t_assert( fBool.At(1L) == NULL );
	t_assert( fBool[1L] == NULL );
}

void AnythingConstructorsTest::LongConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	Anything anyHlp = fLong;

	t_assert( fLong.GetType() == AnyLongType );
	assertCompare( fLong.IsNull(), equal_to, false );
	t_assert( fLong.IsNull() == 0 );
	assertCompare( fLong.GetSize(), equal_to, 1L );
	t_assert( fLong == anyHlp );
	t_assert( !(fLong != anyHlp) );
	t_assert( fLong.IsEqual(anyHlp) );

	t_assert( String("5") == fLong.AsCharPtr() );
	t_assert( String("5") == fLong.AsCharPtr("Default") );
	assertCharPtrEqual( fLong.AsCharPtr(0), "5");
	assertCompare( fLong.AsLong(), equal_to, 5L );
	assertCompare( fLong.AsLong(1234), equal_to, 5L );
	assertCompare( fLong.AsBool(), equal_to, true );
	assertCompare( fLong.AsBool(false), equal_to, true );
	t_assert( fLong.AsBool() == 1 );
	t_assert( fLong.AsDouble() == (double)5 );
	t_assert( fLong.AsDouble(2.3) == (double)5 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( fLong.AsIFAObject(&testObj) == &testObj );
	t_assert( fLong.AsIFAObject(&testObj) != &testObjDummy );
	t_assert( fLong.AsIFAObject() == NULL );

	assertCompare( (fLong.AsString()).Length(), equal_to, 1L );
	t_assert( (fLong.AsString()).Capacity() >= (fLong.AsString()).Length() );
	t_assert( fLong.AsString() == "5" );
	t_assert( fLong.AsString("Default") == "5" );

	assertCompare( fLong.Contains("testSlot"), equal_to, false );
	assertCompare( fLong.FindValue("testSlot"), less, 0L );

	// Does it survive?
	assertCompare( fLong.GetType(), equal_to, AnyLongType );
	fLong.Remove(-1L);
	assertCompare( fLong.GetType(), equal_to, AnyLongType );
	fLong.Remove(0L);
	assertCompare( fLong.GetType(), equal_to, AnyNullType );
	fLong.Remove(1L);
	assertCompare( fLong.FindIndex("testSlots"), less, 0L );
	assertCompare( fLong.IsDefined("testSlots"), equal_to, false );
	t_assert( fLong.SlotName(0) == NULL );
	t_assert( fLong.At(-1L) == fLong.At(0L) );
	assertCompare( fLong.GetType(), equal_to, AnyArrayType );
	assertCompare( fLong[0L].GetType(), equal_to, AnyNullType );
	assertCompare( (fLong.At(0L)).AsLong(-1L), equal_to, -1L );
	t_assert( fLong.At(1L) == NULL );
	assertCharPtrEqual( fLong[0L].AsCharPtr(""), "");
	t_assert( fLong[1L] == NULL );
}

void AnythingConstructorsTest::DoubleConstr0Test() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).
	// fDouble = 7.125;	there is an exact binary representation

	Anything anyHlp = fDouble;

	t_assert( fDouble.GetType() == AnyDoubleType );
	assertCompare( fDouble.IsNull(), equal_to, false );
	assertCompare( fDouble.IsNull(), equal_to, false );
	t_assert( fDouble.IsNull() == 0 );
	assertCompare( fDouble.GetSize(), equal_to, 1L );
	t_assert( fDouble == anyHlp );
	t_assert( !(fDouble != anyHlp) );
	t_assert( fDouble.IsEqual(anyHlp) );

	assertCharPtrEqual( "7.125", fDouble.AsCharPtr() );
	assertCharPtrEqual( "7.125", fDouble.AsCharPtr("Default") );
	assertCharPtrEqual( fDouble.AsCharPtr(0), "7.125");
	assertCompare( fDouble.AsLong(), equal_to, 7L );
	assertCompare( fDouble.AsLong(1234), equal_to, 7L );
	assertCompare( fDouble.AsBool(), equal_to, false );
	assertCompare( fDouble.AsBool(false), equal_to, false );
	t_assert( fDouble.AsBool() == 0 );
	assertCompare( fDouble.AsDouble(), equal_to, 7.125 );
	assertCompare( fDouble.AsDouble(2.3), equal_to, 7.125 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( fDouble.AsIFAObject(&testObj) == &testObj );
	t_assert( fDouble.AsIFAObject(&testObj) != &testObjDummy );
	t_assert( fDouble.AsIFAObject() == NULL );

	assertEqual( 5L, fDouble.AsString().Length() );
	t_assert( (fDouble.AsString()).Capacity() >= (fDouble.AsString()).Length() );
	assertCharPtrEqual( "7.125", fDouble.AsString() );
	assertCharPtrEqual( "7.125", fDouble.AsString("Default") );

	assertCompare( fDouble.Contains("testSlot"), equal_to, false );
	assertCompare( fDouble.FindValue("testSlot"), less, 0L );

	// Does it survive?
	fDouble.Remove(-1L);
	fDouble.Remove(0L);
	fDouble.Remove(1L);
	assertCompare( fDouble.FindIndex("testSlots"), less, 0L );
	assertCompare( fDouble.IsDefined("testSlots"), equal_to, false );
	t_assert( fDouble.SlotName(0) == NULL );
	t_assert( fDouble.At(-1L) == fDouble.At(0L) );
	assertCompare( (fDouble.At(0L)).AsLong(1), equal_to, 1L );
	t_assert( fDouble.At(1L) == NULL );
	assertCharPtrEqual( fDouble[0L].AsCharPtr(""), "");
	t_assert( fDouble[1L] == NULL );
}

void AnythingConstructorsTest::DoubleConstr1Test() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).
	// fDouble2 = 8.1;	there is no exact binary representation

	Anything anyHlp = fDouble2;

	t_assert( fDouble2.GetType() == AnyDoubleType );
	assertCompare( fDouble2.IsNull(), equal_to, false );
	assertCompare( fDouble2.IsNull(), equal_to, false );
	t_assert( fDouble2.IsNull() == 0 );
	assertCompare( fDouble2.GetSize(), equal_to, 1L );
	t_assert( fDouble2 == anyHlp );
	t_assert( !(fDouble2 != anyHlp) );
	t_assert( fDouble2.IsEqual(anyHlp) );

	assertCharPtrEqual( "8.1", fDouble2.AsString().SubString(0, 6) );
	assertCharPtrEqual( "8.1", fDouble2.AsString() );
	assertCharPtrEqual( "8.1", fDouble2.AsString("Default") );
	assertEqual( 8L, fDouble2.AsLong() );
	assertEqual( 8L, fDouble2.AsLong(1234) );
	t_assert( !fDouble2.AsBool() );
	t_assert( fDouble2.AsBool(true) );
	assertDoublesEqual( 8.1, fDouble2.AsDouble(), 0.1 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simplest IFAObject that can be instantiated
	t_assert( fDouble2.AsIFAObject(&testObj) == &testObj );
	t_assert( fDouble2.AsIFAObject(&testObj) != &testObjDummy );
	t_assert( fDouble2.AsIFAObject() == NULL );

	assertCompare( fDouble2.Contains("testSlot"), equal_to, false );
	assertCompare( fDouble2.FindValue("testSlot"), less, 0L );

	// Does it survive?
	fDouble2.Remove(-1L);
	fDouble2.Remove(0L);
	fDouble2.Remove(1L);
	assertCompare( fDouble2.FindIndex("testSlots"), less, 0L );
	assertCompare( fDouble2.IsDefined("testSlots"), equal_to, false );
	t_assert( fDouble2.SlotName(0) == NULL );
	t_assert( fDouble2.At(-1L) == fDouble2.At(0L) );
	assertCompare( (fDouble2.At(0L)).AsLong(-2), equal_to, -2L );
	t_assert( fDouble2.At(1L) == NULL );
	t_assert( fDouble2[1L] == NULL );
}

void AnythingConstructorsTest::FloatConstrTest() // Test if the dafault constructor generates an anything-object with good reactions.
{
	// The method called now are assumed to work correctly (they are being tested later on).

	Anything anyHlp = fFloat;

	t_assert( fFloat.GetType() == AnyDoubleType );
	assertCompare( fFloat.IsNull(), equal_to, false );
	assertCompare( fFloat.IsNull(), equal_to, false );
	t_assert( fFloat.IsNull() == 0 );
	assertCompare( fFloat.GetSize(), equal_to, 1L );
	t_assert( fFloat == anyHlp );
	t_assert( !(fFloat != anyHlp) );
	t_assert( fFloat.IsEqual(anyHlp) );

	// here we use 14 significant digits
	fFloat = -24.490123456789;
	double testDouble = -24.490123456789;
	assertDoublesEqual( testDouble, fFloat.AsDouble(0), 0.000000000001 );
	assertCharPtrEqual( "-24.4901234567", fFloat.AsString().SubString(0, 14) );
	assertCharPtrEqual( "-24.490123456789", fFloat.AsString() );
	assertLongsEqual( -24L, fFloat.AsLong() );
	assertLongsEqual( -24L, fFloat.AsLong(1234) );
	t_assert( !fFloat.AsBool() );
	t_assert( fFloat.AsBool(true) );
	assertEqual( 0, fFloat.AsBool() );
	assertDoublesEqual( (double) - 24.4901, fFloat.AsDouble(), 0.0001 );
	t_assert( fFloat.AsDouble(2.3) == (double) - 24.490123456789 );
	t_assert( fFloat.AsDouble(2.3) != (double) - 24.4901 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( fFloat.AsIFAObject(&testObj) == &testObj );
	t_assert( fFloat.AsIFAObject(&testObj) != &testObjDummy );
	t_assert( fFloat.AsIFAObject() == NULL );

	assertEqual( 16, fFloat.AsString().Length() );
	assertCharPtrEqual( "-24.490123456789", fFloat.AsString("Default") );

	t_assert( !fFloat.Contains("testSlot") );
	assertCompare( fFloat.FindValue("testSlot"), less, 0L );

	// Does it survive?
	fFloat.Remove(-1L);
	fFloat.Remove(0L);
	fFloat.Remove(1L);
	assertCompare( fFloat.FindIndex("testSlots"), less, 0L );
	assertCompare( fFloat.IsDefined("testSlots"), equal_to, false );
	t_assert( fFloat.SlotName(0) == NULL );
	t_assert( fFloat.At(-1L) == fFloat.At(0L) );
	t_assert( (fFloat.At(0L)).AsLong(3L) == 3L );
	t_assert( fFloat.At(1L) == NULL );
	t_assert( fFloat[1L] == NULL );

	Anything anyTest = -24.51;
	t_assert( anyTest.AsLong() == -24L );
}

void AnythingConstructorsTest::CharStarConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	char charStarTest[50] = { 0 };
	memcpy(charStarTest, "A String", strlen("A String"));

	Anything anyCharStar(charStarTest), anyHlp = fString;

	t_assert( anyCharStar.GetType() == AnyCharPtrType );
	assertCompare( anyCharStar.IsNull(), equal_to, false );
	assertCompare( anyCharStar.IsNull(), equal_to, false );
	t_assert( anyCharStar.IsNull() == 0 );
	assertCompare( anyCharStar.GetSize(), equal_to, 1L );
	t_assert( anyCharStar == anyHlp );
	t_assert( !(anyCharStar != anyHlp) );
	t_assert( anyCharStar.IsEqual(anyHlp) );

	t_assert( String("A String") == anyCharStar.AsCharPtr() );
	t_assert( String("A String") == anyCharStar.AsCharPtr("Default") );
	assertCharPtrEqual( anyCharStar.AsCharPtr(0), "A String");
	assertCompare( anyCharStar.AsLong(), equal_to, 0L );
	assertCompare( anyCharStar.AsLong(1234), equal_to, 1234L );
	assertEqual ( 1234L, anyCharStar.AsLong(1234) );
	assertCompare( anyCharStar.AsBool(), equal_to, false );
	assertCompare( anyCharStar.AsBool(true), equal_to, true );
	t_assert( anyCharStar.AsBool() == 0 );
	t_assert( anyCharStar.AsDouble() == (double)0 );
	t_assert( anyCharStar.AsDouble(2.3) == (double)2.3 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( anyCharStar.AsIFAObject(&testObj) == &testObj );
	t_assert( anyCharStar.AsIFAObject(&testObj) != &testObjDummy );
	t_assert( anyCharStar.AsIFAObject() == NULL );

	t_assert( (anyCharStar.AsString()).Length() == (long)strlen("A String") );
	t_assert( (anyCharStar.AsString()).Capacity() >= (anyCharStar.AsString()).Length() );
	t_assert( anyCharStar.AsString() == "A String" );
	t_assert( anyCharStar.AsString("Default") == "A String" );

	assertCompare( anyCharStar.Contains("testSlot"), equal_to, false );
	assertCompare( anyCharStar.FindValue("testSlot"), less, 0L );

	// Does it survive?
	anyCharStar.Remove(-1L);
	anyCharStar.Remove(0L);
	anyCharStar.Remove(1L);
	assertCompare( anyCharStar.FindIndex("testSlots"), less, 0L );
	assertCompare( anyCharStar.IsDefined("testSlots"), equal_to, false );
	t_assert( anyCharStar.SlotName(0) == NULL );
	t_assert( anyCharStar.At(-1L) == anyCharStar.At(0L) );
	t_assert( (anyCharStar.At(0L)).AsLong() == 0 );
	t_assert( anyCharStar.At(1L) == NULL );
	assertCharPtrEqual( anyCharStar[0L].AsCharPtr(""), "" );
	t_assert( anyCharStar[1L] == NULL );
}

void AnythingConstructorsTest::CharStarLongConstr0Test() {
	Anything anyStringLen("abcdefgh", 8L), anyHlp;

	anyHlp = anyStringLen;

	t_assert( anyStringLen.GetType() == AnyCharPtrType );
	assertCompare( anyStringLen.IsNull(), equal_to, false );
	assertCompare( anyStringLen.IsNull(), equal_to, false );
	t_assert( anyStringLen.IsNull() == 0 );
	assertCompare( anyStringLen.GetSize(), equal_to, 1L );
	t_assert( anyStringLen == anyHlp );
	t_assert( !(anyStringLen != anyHlp) );
	t_assert( anyStringLen.IsEqual(anyHlp) );

	t_assert( String("abcdefgh") == anyStringLen.AsCharPtr() );
	t_assert( String("abcdefgh") == anyStringLen.AsCharPtr("Default") );
	assertCharPtrEqual( anyStringLen.AsCharPtr(0), "abcdefgh");

	assertCompare( anyStringLen.AsLong(), equal_to, 0L );
	assertCompare( anyStringLen.AsLong(1234), equal_to, 1234L );
	assertCompare( anyStringLen.AsBool(), equal_to, false );
	assertCompare( anyStringLen.AsBool(true), equal_to, true );
	t_assert( anyStringLen.AsBool() == 0 );
	t_assert( anyStringLen.AsDouble() == (double)0 );
	t_assert( anyStringLen.AsDouble(2.3) == (double)2.3 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( anyStringLen.AsIFAObject(&testObj) == &testObj );
	t_assert( anyStringLen.AsIFAObject(&testObj) != &testObjDummy );
	t_assert( anyStringLen.AsIFAObject() == NULL );

	t_assert( (anyStringLen.AsString()).Length() == (long)strlen("abcdefgh") );
	t_assert( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	t_assert( (anyStringLen.AsString()).Capacity() >= (anyStringLen.AsString(0)).Length() );
	t_assert( anyStringLen.AsString() == "abcdefgh" );
	t_assert( anyStringLen.AsString("Default") == "abcdefgh" );

	assertCompare( anyStringLen.Contains("testSlot"), equal_to, false );
	assertCompare( anyStringLen.FindValue("testSlot"), less, 0L );

	// Does it survive?
	anyStringLen.Remove(-1L);
	anyStringLen.Remove(0L);
	anyStringLen.Remove(1L);
	assertCompare( anyStringLen.FindIndex("testSlots"), less, 0L );
	assertCompare( anyStringLen.IsDefined("testSlots"), equal_to, false );
	t_assert( anyStringLen.SlotName(0) == NULL );
	t_assert( anyStringLen.At(-1L) == anyStringLen.At(0L) );
	t_assert( (anyStringLen.At(0L)).AsLong() == 0 );
	t_assert( anyStringLen.At(1L) == NULL );
	assertCharPtrEqual( anyStringLen[0L].AsCharPtr(""), "");
	t_assert( anyStringLen[1L] == NULL );
}

void AnythingConstructorsTest::CharStarLongConstr1Test() {
	Anything anyStringLen("abcdefgh", 4L), anyHlp;

	anyHlp = anyStringLen;

	t_assert( anyStringLen.GetType() == AnyCharPtrType );
	assertCompare( anyStringLen.IsNull(), equal_to, false );
	assertCompare( anyStringLen.IsNull(), equal_to, false );
	t_assert( anyStringLen.IsNull() == 0 );
	assertCompare( anyStringLen.GetSize(), equal_to, 1L );
	t_assert( anyStringLen == anyHlp );
	t_assert( !(anyStringLen != anyHlp) );
	t_assert( anyStringLen.IsEqual(anyHlp) );

	t_assert( String("abcd") == anyStringLen.AsCharPtr() );
	t_assert( String("abcd") == anyStringLen.AsCharPtr("Default") );
	assertCharPtrEqual( anyStringLen.AsCharPtr(0), "abcd");

	assertCompare( anyStringLen.AsLong(), equal_to, 0L );

	assertCompare( anyStringLen.AsLong(1234), equal_to, 1234L );

	assertCompare( anyStringLen.AsBool(), equal_to, false );
	assertCompare( anyStringLen.AsBool(true), equal_to, true );
	t_assert( anyStringLen.AsBool() == 0 );
	t_assert( anyStringLen.AsDouble() == (double)0 );
	t_assert( anyStringLen.AsDouble(2.3) == (double)2.3 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( anyStringLen.AsIFAObject(&testObj) == &testObj );
	t_assert( anyStringLen.AsIFAObject(&testObj) != &testObjDummy );
	t_assert( anyStringLen.AsIFAObject() == NULL );

	t_assert( (anyStringLen.AsString()).Length() == (long)strlen("abcd") );
	t_assert( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	t_assert( (anyStringLen.AsString()).Capacity() >= (anyStringLen.AsString(0)).Length() );
	t_assert( anyStringLen.AsString() == "abcd" );
	t_assert( anyStringLen.AsString("Default") == "abcd" );

	assertCompare( anyStringLen.Contains("testSlot"), equal_to, false );
	assertCompare( anyStringLen.FindValue("testSlot"), less, 0L );

	// Does it survive?
	anyStringLen.Remove(-1L);
	anyStringLen.Remove(0L);
	anyStringLen.Remove(1L);
	assertCompare( anyStringLen.FindIndex("testSlots"), less, 0L );
	assertCompare( anyStringLen.IsDefined("testSlots"), equal_to, false );
	t_assert( anyStringLen.SlotName(0) == NULL );
	t_assert( anyStringLen.At(-1L) == anyStringLen.At(0L) );
	t_assert( (anyStringLen.At(0L)).AsLong() == 0 );
	t_assert( anyStringLen.At(1L) == NULL );
	assertCharPtrEqual( anyStringLen[0L].AsCharPtr(""), "");
	t_assert( anyStringLen[1L] == NULL );
}

void AnythingConstructorsTest::CharStarLongConstr2Test() {
	// Frage:  Ist der Inhalt gleich wegen Zufall oder wird festgestellt, dass strlen("abcdefgh") < 10 ist?
#define TSTSTR "abcdefgh"
	Anything anyStringLen(TSTSTR, 10L);
	// PT: this is not nice: constructor will read past the end of the string,
	//     it might crash!
	long l = anyStringLen.AsString().Length();

	t_assert( l == (long)strlen(TSTSTR) ); // this would also be a reasonable assertion
	// assertLongsEqual( 10, l );				// semantic change, above is correct!

	// t_assert( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	// the above would be reasonable also
	assertCharPtrEqual( anyStringLen.AsCharPtr(0), "abcdefgh");
}

void AnythingConstructorsTest::CharStarLongConstr3Test() {
	// negative Werte bedeuten "Ich kenne die Laenge nicht" --> Die Laenge wird die Laenge der Zeichenkette

	Anything anyStringLen("abcdefgh", -3L);

	t_assert( (anyStringLen.AsString()).Length() == (long)strlen("abcdefgh") );
	t_assert( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	t_assert( anyStringLen.AsString() == "abcdefgh" );

	t_assert( (anyStringLen.AsString()).Length() <= (anyStringLen.AsString()).Capacity() );
}

void AnythingConstructorsTest::StringConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	String stringTest = "A String";
	Anything anyString(stringTest), anyHlp = fString;

	t_assert( anyString.GetType() == AnyCharPtrType );
	assertCompare( anyString.IsNull(), equal_to, false );
	assertCompare( anyString.IsNull(), equal_to, false );
	t_assert( anyString.IsNull() == 0 );
	assertCompare( anyString.GetSize(), equal_to, 1L );
	t_assert( anyString.AsString() == anyHlp.AsString() );
	t_assert( !(anyString.AsString() != anyHlp.AsString()) );
	t_assert( anyString.IsEqual(anyHlp) );

	assertCharPtrEqual( anyString.AsCharPtr(0), "A String");
	assertCharPtrEqual( anyString.AsCharPtr("Default"), "A String");
	t_assert( anyString.AsLong() == 0L );
	assertCompare( anyString.AsLong(1234), equal_to, 1234L );

	assertCompare( anyString.AsBool(), equal_to, false );
	assertCompare( anyString.AsBool(true), equal_to, true );
	t_assert( anyString.AsBool() == 0 );
	t_assert( anyString.AsDouble() == (double)0 );
	t_assert( anyString.AsDouble(2.3) == (double)2.3 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( anyString.AsIFAObject(&testObj) == &testObj );
	t_assert( anyString.AsIFAObject(&testObj) != &testObjDummy );
	t_assert( anyString.AsIFAObject() == NULL );

	t_assert( (anyString.AsString()).Length() == (long)strlen("A String") );
	t_assert( (anyString.AsString()).Capacity() >= (anyString.AsString()).Length() );
	t_assert( anyString.AsString() == "A String" );
	t_assert( anyString.AsString("Default") == "A String" );

	String stringHlp;
	stringHlp = anyString.AsString();
	t_assert( stringHlp == "A String" );
	stringHlp = String(anyString.AsString());
	t_assert( stringHlp == "A String" );

	assertCompare( anyString.Contains("testSlot"), equal_to, false );
	assertCompare( anyString.FindValue("testSlot"), less, 0L );

	// Does it survive?
	anyString.Remove(-1L);
	anyString.Remove(0L);
	anyString.Remove(1L);
	assertCompare( anyString.FindIndex("testSlots"), less, 0L );
	assertCompare( anyString.IsDefined("testSlots"), equal_to, false );
	t_assert( anyString.SlotName(0) == NULL );
	t_assert( anyString.At(-1L) == anyString.At(0L) );
	t_assert( (anyString.At(0L)).AsLong() == 0L );
	t_assert( anyString.At(1L) == NULL );
	assertCharPtrEqual( anyString[0L].AsCharPtr(""), "");
	t_assert( anyString[1L] == NULL );

	String voidstr((void *) "abc\0ef", 5); // string with 0 byte included
	Anything avoidstr(voidstr);
	t_assert(avoidstr.GetType() == AnyCharPtrType);
	t_assert(avoidstr.AsString() == voidstr); // does it remain the same
	assertEqual(voidstr.Length(), avoidstr.AsString().Length());
}

void AnythingConstructorsTest::EmptyVoidStarLenConstrTest() {
	char test[10];
	memset(test, '\0', 10);
	Anything anyTest((void *) 0, 10);

	t_assert( anyTest.GetType() == AnyVoidBufType );
	assertCompare( anyTest.IsNull(), equal_to, false );
	assertCompare( anyTest.GetSize(), equal_to, 1L );
	t_assert( anyTest.AsCharPtr(0) != 0 );
	assertCharPtrEqual( (const char *) test, anyTest.AsCharPtr());

	Anything anyTest1((void *) test, 0); // we do not allocate something
	t_assert( anyTest1.GetType() == AnyVoidBufType );
	assertCompare( anyTest1.IsNull(), equal_to, false );
	assertCompare( anyTest1.GetSize(), equal_to, 1L );
	t_assert( anyTest1.AsCharPtr(0) == 0 );

	test[0] = '1';
	test[5] = '6';
	Anything anyTest2((void *) test, 10);
	t_assert( anyTest2.GetType() == AnyVoidBufType );
	assertCompare( anyTest2.IsNull(), equal_to, false );
	assertCompare( anyTest2.GetSize(), equal_to, 1L );
	t_assert( anyTest2.AsCharPtr(0) != 0 );
	assertCharPtrEqual( (const char *) test, anyTest2.AsCharPtr());
}

void AnythingConstructorsTest::VoidStarLenConstrTest() {
	typedef long arrValueType;
	arrValueType arrTest[5] = { 0, 1, 2, 3, 4 };
	Anything anyTest((void *) arrTest, static_cast<long>(sizeof(arrTest)));
	size_t _byteLengthOfArray = sizeof(arrTest);
	long _elementsInArray = _byteLengthOfArray/sizeof(arrValueType);
	Anything anyHlp = anyTest;

	t_assert( anyTest.GetType() == AnyVoidBufType );
	assertCompare( anyTest.IsNull(), equal_to, false );
	assertCompare( anyTest.IsNull(), equal_to, false );
	t_assert( anyTest.IsNull() == 0 );
	assertCompare( anyTest.GetSize(), equal_to, 1L );
	t_assert( anyTest == anyHlp );
	t_assert( !(anyTest != anyHlp) );
	t_assert( anyTest.IsEqual(anyHlp) );

	assertCharPtrEqual( (const char *) arrTest, anyTest.AsCharPtr());
	// AsCharPtr returns the address of the buffer of the binary any
	t_assert( anyTest.AsLong() != 0 ); // address of the buffer (also)
	assertEqual((long) anyTest.AsCharPtr(0), anyTest.AsLong());
	assertCompare( anyTest.AsBool(), equal_to, false );
	assertCompare( anyTest.AsBool(false), equal_to, false );
	t_assert( anyTest.AsDouble() == (double)0 );
	assertCompare( anyTest.AsDouble(2.3), equal_to, 2.3 );

	DummyIFAObj testObj("Test");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( anyTest.AsIFAObject(&testObj) == &testObj );
	t_assert( anyTest.AsIFAObject() == NULL );

	assertEqual( (long)sizeof(arrTest), (anyTest.AsString()).Length() );
	t_assert( (anyTest.AsString()).Capacity() >= (anyTest.AsString()).Length() );
	t_assert( anyTest.AsString() == String( (void *)arrTest, (long)sizeof(arrTest) ) );
	t_assert( anyTest.AsString("Default") == String( (void *)arrTest, (long)sizeof(arrTest) ) );

	long i;
	for (i = 0; i < _elementsInArray; i++) {
		t_assert ( ( (long *)((const char *)anyTest.AsString()) )[i] == i );
		t_assert ( ( (long *)((const char *)anyTest.At(0L).AsString()) )[i] == i );
		t_assert ( ( (long *)((const char *)anyTest[0L].AsString()) )[i] == i );

		t_assert ( ( (long *)(anyTest.AsCharPtr()) )[i] == i );
		t_assert ( ( (long *)(anyTest.At(0L).AsCharPtr()) )[i] == i );
		t_assert ( ( (long *)((const char *)anyTest[0L].AsString()) )[i] == i );
	}

	assertCompare( anyTest.Contains("testSlot"), equal_to, false );
	assertCompare( anyTest.FindValue("testSlot"), less, 0L );

	// Does it survive?
	assertEqual(_byteLengthOfArray, anyTest[0L].AsString().Length());
	assertEqual("", anyTest[0L].AsString());
	anyTest.Remove(-1L);
	assertEqual(1, anyTest.GetSize());
	anyTest.Remove(0L);
	assertEqual(0, anyTest.GetSize());
	anyTest.Remove(1L);
	assertEqual(0, anyTest.GetSize());
	assertCompare( anyTest.FindIndex("testSlots"), less, 0L );
	assertCompare( anyTest.IsDefined("testSlots"), equal_to, false );
	t_assert( anyTest.SlotName(0) == NULL );
	t_assert( anyTest.At(-1L) == anyTest.At(0L) );
	assertCompare( (anyTest.At(0L)).AsLong(-1L), equal_to, -1L );
	t_assert( anyTest.At(1L) == NULL );
	assertCharPtrEqual( anyTest[0L].AsCharPtr(""), (char *)arrTest);
	t_assert( anyTest[1L] == NULL );
}

void AnythingConstructorsTest::IFAObjectStarConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	String testAdr;
	testAdr << (long) &testObj; // the address of the object for comparison

	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	Anything anyIFAObj(&testObj), anyHlp = anyIFAObj;

	t_assert( anyIFAObj.GetType() == AnyObjectType );
	assertCompare( anyIFAObj.IsNull(), equal_to, false );
	assertCompare( anyIFAObj.IsNull(), equal_to, false );
	t_assert( anyIFAObj.IsNull() == 0 );
	assertCompare( anyIFAObj.GetSize(), equal_to, 1L );
	t_assert( anyIFAObj == anyHlp );
	t_assert( !(anyIFAObj != anyHlp) );
	t_assert( anyIFAObj.IsEqual(anyHlp) );

	t_assert( String("IFAObject") == anyIFAObj.AsCharPtr() ); // Ist es OK ????
	t_assert( String("IFAObject") == anyIFAObj.AsCharPtr("Default") );
	assertCharPtrEqual( anyIFAObj.AsCharPtr(0), "IFAObject");
	t_assert( anyIFAObj.AsLong() != 0L ); // address of object
	t_assert( anyIFAObj.AsLong(1234) != 1234L );
	assertCompare( anyIFAObj.AsBool(), equal_to, false );
	assertCompare( anyIFAObj.AsBool(false), equal_to, false );
	t_assert( anyIFAObj.AsDouble() == (double)0 );
	assertCompare( anyIFAObj.AsDouble(2.3), equal_to, 2.3 );

	t_assert( anyIFAObj.AsIFAObject(&testObj) == &testObj );
	t_assert( anyIFAObj.AsIFAObject(&testObj) != &testObjDummy );
	t_assert( anyIFAObj.AsIFAObject() == &testObj );

	// PT: somewhat inconsistent, AsString returns the address, AsCharPtr the type
	// PS: now it is consistent so the testcases fail
	t_assert( String("IFAObject") == anyIFAObj.AsString() ); // Ist es OK ????
	t_assert( (anyIFAObj.AsString()).Capacity() >= (anyIFAObj.AsString()).Length() );

	t_assert( (anyIFAObj.At(0L)).AsLong() == (long)&testObj );
	// returns the address
	t_assert( anyIFAObj.At(1L) == NULL );

	assertCharPtrEqual( anyIFAObj[0L].AsCharPtr(), "IFAObject");
	assertCompare( anyIFAObj.Contains("testSlot"), equal_to, false );
	assertCompare( anyIFAObj.FindValue("testSlot"), less, 0L );

	// Does it survive?
	anyIFAObj.Remove(-1L);

	assertCompare( anyIFAObj.At(-1L).GetType(), equal_to, AnyObjectType );

	t_assert( (anyIFAObj.At(0L)).AsLong() == (long)&testObj );
	t_assert( anyIFAObj.At(1L) == NULL );
	// PT: Probably this statement switches to an ArrayImpl
	assertLongsEqual(anyIFAObj.GetSize(), 2 );
	t_assert(anyIFAObj[0L].AsIFAObject() == &testObj );
	t_assert( anyIFAObj[1L] == NULL );
	//t_assert( anyIFAObj[0L].AsString() == testAdr );
	assertCharPtrEqual( "IFAObject", anyIFAObj[0L].AsCharPtr() );

	anyIFAObj.Remove(1L);
	assertCompare( anyIFAObj.FindIndex("testSlots"), less, 0L );
	assertCompare( anyIFAObj.IsDefined("testSlots"), equal_to, false );
	t_assert( anyIFAObj.SlotName(0) == NULL );
	t_assert( anyIFAObj[1L] == NULL );

	anyIFAObj.Remove(0L);
	assertCompare( (anyIFAObj.At(0L)).AsLong(), equal_to, 0L );
	assertCompare( anyIFAObj[0L].GetType(), equal_to, AnyNullType );
}

void AnythingConstructorsTest::AnythingConstrTest() {
	Anything any0;
	Anything anyTest0(any0);
	anyTest0.IsEqual(any0);
	any0.IsEqual(anyTest0);

	Anything any1(0L);
	Anything anyTest1(any1);
	anyTest1.IsEqual(any1);
	any1.IsEqual(anyTest1);

	Anything any2(true);
	Anything anyTest2(any2);
	anyTest2.IsEqual(any2);
	any2.IsEqual(anyTest2);

	Anything any3((long) 2);
	Anything anyTest3(any3);
	anyTest3.IsEqual(any3);
	any3.IsEqual(anyTest3);

	Anything any4((float) 4.1);
	Anything anyTest4(any4);
	anyTest4.IsEqual(any4);
	any4.IsEqual(anyTest4);

	Anything any5((double) 5.2);
	Anything anyTest5(any5);
	anyTest5.IsEqual(any5);
	any5.IsEqual(anyTest5);

	Anything any6("0123456789");
	Anything anyTest6(any6);
	anyTest6.IsEqual(any6);
	any6.IsEqual(anyTest6);

	Anything any7("abcdefgh", 8);
	Anything anyTest7(any7);
	anyTest7.IsEqual(any7);
	any7.IsEqual(anyTest7);

	Anything any8(String("Anything Test"));
	Anything anyTest8(any8);
	anyTest8.IsEqual(any8);
	any8.IsEqual(anyTest8);

	long buf[5] = { 0, 1, 2, 3, 4 };
	Anything any9((void *) &buf[0], (long) sizeof(buf));
	Anything anyTest9(any9);
	anyTest9.IsEqual(any9);
	any9.IsEqual(anyTest9);

	DummyIFAObj testObj("Test");
	// We use a DummyIFAObj, it is the simplest IFAObject that can be instantiated
	Anything any10(&testObj);
	Anything anyTest10(any10);
	anyTest10.IsEqual(any10);
	any10.IsEqual(anyTest10);
}
