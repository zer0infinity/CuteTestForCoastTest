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

#include "AnythingConstructorsTest.h"
#include "IFAObject.h"
#include "Tracer.h"

AnythingConstructorsTest::AnythingConstructorsTest() : fString("A String"), fLong(5L), fBool(true), fDouble(7.125), fDouble2(8.1), fFloat(-24.490123456789), fNull(Anything()) {
	StartTrace(AnythingConstructorsTest.setUp);
}

void AnythingConstructorsTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, DefaultConstrTest));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, IntConstrTest));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, LongConstrTest));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, FloatConstrTest));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, DoubleConstr0Test));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, DoubleConstr1Test));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, CharStarConstrTest));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, CharStarLongConstr0Test));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, CharStarLongConstr1Test));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, CharStarLongConstr2Test));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, CharStarLongConstr3Test));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, StringConstrTest));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, VoidStarLenConstrTest));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, EmptyVoidStarLenConstrTest));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, IFAObjectStarConstrTest));
	s.push_back(CUTE_SMEMFUN(AnythingConstructorsTest, AnythingConstrTest));
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

	ASSERT( fNull.GetType() == AnyNullType );
	ASSERT_EQUAL( fNull.IsNull(), true );
	ASSERT_EQUAL( fNull.IsNull(), true );
	ASSERT( fNull.IsNull() == bool(1) );
	ASSERT( fNull.GetSize() == bool(0) );
	ASSERT( fNull == anyHlp );
	ASSERT( !(fNull != anyHlp) );
	ASSERT( fNull.IsEqual(anyHlp) );

	ASSERT( String("") == fNull.AsCharPtr() );
	ASSERT( String("Default") == fNull.AsCharPtr("Default") );
	ASSERT( fNull.AsCharPtr(0) == NULL );
	ASSERT_EQUAL( fNull.AsLong(), 0L );
	ASSERT_EQUAL( fNull.AsLong(1234), 1234L );
	ASSERT_EQUAL( fNull.AsBool(), false );
	ASSERT_EQUAL( fNull.AsBool(true), true );
	ASSERT( fNull.AsBool() == 0 );
	ASSERT( fNull.AsDouble() == 0 );
	ASSERT_EQUAL( fNull.AsDouble(2.3), 2.3 );
	ASSERT( fNull.AsIFAObject() == NULL );

	DummyIFAObj testObj("testObj");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	ASSERT( fNull.AsIFAObject(&testObj) == &testObj );

	ASSERT_EQUAL( (fNull.AsString()).Length(), 0L );
	ASSERT( (fNull.AsString()).Capacity() >= (fNull.AsString()).Length() );
	ASSERT( fNull.AsString() == "" );
	ASSERT( fNull.AsString("Default") == "Default" );

	ASSERT( fNull.Contains("testSlot") == false );
	ASSERT_LESS( fNull.FindValue("testSlot"), 0L );

	// Does it survive?
	fNull.Remove(-1L);
	fNull.Remove(0L);
	fNull.Remove(1L);
	ASSERT_LESS( fNull.FindIndex("testSlots"), 0L );
	ASSERT( fNull.IsDefined("testSlots") == false );
	ASSERT( fNull.SlotName(0) == NULL );
	//ASSERT( fNull.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	ASSERT( fNull.At(0L) == NULL );
	ASSERT( fNull.At(1L) == NULL );
	//ASSERT( fNull[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	ASSERT( fNull[0L] == NULL );
	ASSERT( fNull[1L] == NULL );
}

void AnythingConstructorsTest::IntConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	Anything anyHlp = fBool;

	ASSERT( fBool.GetType() == AnyLongType );
	ASSERT_EQUAL( fBool.IsNull(), false );
	ASSERT_EQUAL( fBool.IsNull(), false );
	ASSERT( fBool.IsNull() == 0 );
	ASSERT_EQUAL( fBool.GetSize(), 1L );
	ASSERT( fBool == anyHlp );
	ASSERT( !(fBool != anyHlp) );
	ASSERT( fBool.IsEqual(anyHlp) );

	ASSERT_EQUAL( fBool.AsBool(), true );
	ASSERT_EQUAL( fBool.AsBool(false), true );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	ASSERT( fBool.AsIFAObject(&testObj) == &testObj );
	ASSERT( fBool.AsIFAObject(&testObj) != &testObjDummy );
	ASSERT( fBool.AsIFAObject() == NULL );

	ASSERT_EQUAL( (fBool.AsString()).Length(), 1L );
	ASSERT( (fBool.AsString()).Capacity() >= (fBool.AsString()).Length() );

	ASSERT_EQUAL( fBool.Contains("testSlot"), false );
	ASSERT_LESS( fBool.FindValue("testSlot"), 0L );

	// Does it survive?
	fBool.Remove(-1L);
	fBool.Remove(0L);
	fBool.Remove(1L);
	ASSERT_LESS( fBool.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( fBool.IsDefined("testSlots"), false );
	ASSERT( fBool.SlotName(0) == NULL );
	ASSERT_EQUAL( (fBool.At(0L)).AsBool(false), false );
	ASSERT( fBool.At(1L) == NULL );
	ASSERT( fBool[1L] == NULL );
}

void AnythingConstructorsTest::LongConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	Anything anyHlp = fLong;

	ASSERT( fLong.GetType() == AnyLongType );
	ASSERT_EQUAL( fLong.IsNull(), false );
	ASSERT( fLong.IsNull() == 0 );
	ASSERT_EQUAL( fLong.GetSize(), 1L );
	ASSERT( fLong == anyHlp );
	ASSERT( !(fLong != anyHlp) );
	ASSERT( fLong.IsEqual(anyHlp) );

	ASSERT( String("5") == fLong.AsCharPtr() );
	ASSERT( String("5") == fLong.AsCharPtr("Default") );
	ASSERT_EQUAL( fLong.AsCharPtr(0), "5");
	ASSERT_EQUAL( fLong.AsLong(), 5L );
	ASSERT_EQUAL( fLong.AsLong(1234), 5L );
	ASSERT_EQUAL( fLong.AsBool(), true );
	ASSERT_EQUAL( fLong.AsBool(false), true );
	ASSERT( fLong.AsBool() == 1 );
	ASSERT( fLong.AsDouble() == (double)5 );
	ASSERT( fLong.AsDouble(2.3) == (double)5 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	ASSERT( fLong.AsIFAObject(&testObj) == &testObj );
	ASSERT( fLong.AsIFAObject(&testObj) != &testObjDummy );
	ASSERT( fLong.AsIFAObject() == NULL );

	ASSERT_EQUAL( (fLong.AsString()).Length(), 1L );
	ASSERT( (fLong.AsString()).Capacity() >= (fLong.AsString()).Length() );
	ASSERT( fLong.AsString() == "5" );
	ASSERT( fLong.AsString("Default") == "5" );

	ASSERT_EQUAL( fLong.Contains("testSlot"), false );
	ASSERT_LESS( fLong.FindValue("testSlot"), 0L );

	// Does it survive?
	ASSERT_EQUAL( fLong.GetType(), AnyLongType );
	fLong.Remove(-1L);
	ASSERT_EQUAL( fLong.GetType(), AnyLongType );
	fLong.Remove(0L);
	ASSERT_EQUAL( fLong.GetType(), AnyNullType );
	fLong.Remove(1L);
	ASSERT_LESS( fLong.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( fLong.IsDefined("testSlots"), false );
	ASSERT( fLong.SlotName(0) == NULL );
	ASSERT( fLong.At(-1L) == fLong.At(0L) );
	ASSERT_EQUAL( fLong.GetType(), AnyArrayType );
	ASSERT_EQUAL( fLong[0L].GetType(), AnyNullType );
	ASSERT_EQUAL( (fLong.At(0L)).AsLong(-1L), -1L );
	ASSERT( fLong.At(1L) == NULL );
	ASSERT_EQUAL( fLong[0L].AsCharPtr(""), "");
	ASSERT( fLong[1L] == NULL );
}

void AnythingConstructorsTest::DoubleConstr0Test() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).
	// fDouble = 7.125;	there is an exact binary representation

	Anything anyHlp = fDouble;

	ASSERT( fDouble.GetType() == AnyDoubleType );
	ASSERT_EQUAL( fDouble.IsNull(), false );
	ASSERT_EQUAL( fDouble.IsNull(), false );
	ASSERT( fDouble.IsNull() == 0 );
	ASSERT_EQUAL( fDouble.GetSize(), 1L );
	ASSERT( fDouble == anyHlp );
	ASSERT( !(fDouble != anyHlp) );
	ASSERT( fDouble.IsEqual(anyHlp) );

	ASSERT_EQUAL( "7.125", fDouble.AsCharPtr() );
	ASSERT_EQUAL( "7.125", fDouble.AsCharPtr("Default") );
	ASSERT_EQUAL( fDouble.AsCharPtr(0), "7.125");
	ASSERT_EQUAL( fDouble.AsLong(), 7L );
	ASSERT_EQUAL( fDouble.AsLong(1234), 7L );
	ASSERT_EQUAL( fDouble.AsBool(), false );
	ASSERT_EQUAL( fDouble.AsBool(false), false );
	ASSERT( fDouble.AsBool() == 0 );
	ASSERT_EQUAL( fDouble.AsDouble(), 7.125 );
	ASSERT_EQUAL( fDouble.AsDouble(2.3), 7.125 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	ASSERT( fDouble.AsIFAObject(&testObj) == &testObj );
	ASSERT( fDouble.AsIFAObject(&testObj) != &testObjDummy );
	ASSERT( fDouble.AsIFAObject() == NULL );

	ASSERT_EQUAL( 5L, fDouble.AsString().Length() );
	ASSERT( (fDouble.AsString()).Capacity() >= (fDouble.AsString()).Length() );
	ASSERT_EQUAL( "7.125", fDouble.AsString() );
	ASSERT_EQUAL( "7.125", fDouble.AsString("Default") );

	ASSERT_EQUAL( fDouble.Contains("testSlot"), false );
	ASSERT_LESS( fDouble.FindValue("testSlot"), 0L );

	// Does it survive?
	fDouble.Remove(-1L);
	fDouble.Remove(0L);
	fDouble.Remove(1L);
	ASSERT_LESS( fDouble.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( fDouble.IsDefined("testSlots"), false );
	ASSERT( fDouble.SlotName(0) == NULL );
	ASSERT( fDouble.At(-1L) == fDouble.At(0L) );
	ASSERT_EQUAL( (fDouble.At(0L)).AsLong(1), 1L );
	ASSERT( fDouble.At(1L) == NULL );
	ASSERT_EQUAL( fDouble[0L].AsCharPtr(""), "");
	ASSERT( fDouble[1L] == NULL );
}

void AnythingConstructorsTest::DoubleConstr1Test() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).
	// fDouble2 = 8.1;	there is no exact binary representation

	Anything anyHlp = fDouble2;

	ASSERT( fDouble2.GetType() == AnyDoubleType );
	ASSERT_EQUAL( fDouble2.IsNull(), false );
	ASSERT_EQUAL( fDouble2.IsNull(), false );
	ASSERT( fDouble2.IsNull() == 0 );
	ASSERT_EQUAL( fDouble2.GetSize(), 1L );
	ASSERT( fDouble2 == anyHlp );
	ASSERT( !(fDouble2 != anyHlp) );
	ASSERT( fDouble2.IsEqual(anyHlp) );

	ASSERT_EQUAL( "8.1", fDouble2.AsString().SubString(0, 6) );
	ASSERT_EQUAL( "8.1", fDouble2.AsString() );
	ASSERT_EQUAL( "8.1", fDouble2.AsString("Default") );
	ASSERT_EQUAL( 8L, fDouble2.AsLong() );
	ASSERT_EQUAL( 8L, fDouble2.AsLong(1234) );
	ASSERT( !fDouble2.AsBool() );
	ASSERT( fDouble2.AsBool(true) );
	ASSERT_EQUAL_DELTA( 8.1, fDouble2.AsDouble(), 0.1 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simplest IFAObject that can be instantiated
	ASSERT( fDouble2.AsIFAObject(&testObj) == &testObj );
	ASSERT( fDouble2.AsIFAObject(&testObj) != &testObjDummy );
	ASSERT( fDouble2.AsIFAObject() == NULL );

	ASSERT_EQUAL( fDouble2.Contains("testSlot"), false );
	ASSERT_LESS( fDouble2.FindValue("testSlot"), 0L );

	// Does it survive?
	fDouble2.Remove(-1L);
	fDouble2.Remove(0L);
	fDouble2.Remove(1L);
	ASSERT_LESS( fDouble2.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( fDouble2.IsDefined("testSlots"), false );
	ASSERT( fDouble2.SlotName(0) == NULL );
	ASSERT( fDouble2.At(-1L) == fDouble2.At(0L) );
	ASSERT_EQUAL( (fDouble2.At(0L)).AsLong(-2), -2L );
	ASSERT( fDouble2.At(1L) == NULL );
	ASSERT( fDouble2[1L] == NULL );
}

void AnythingConstructorsTest::FloatConstrTest() // Test if the dafault constructor generates an anything-object with good reactions.
{
	// The method called now are assumed to work correctly (they are being tested later on).

	Anything anyHlp = fFloat;

	ASSERT( fFloat.GetType() == AnyDoubleType );
	ASSERT_EQUAL( fFloat.IsNull(), false );
	ASSERT_EQUAL( fFloat.IsNull(), false );
	ASSERT( fFloat.IsNull() == 0 );
	ASSERT_EQUAL( fFloat.GetSize(), 1L );
	ASSERT( fFloat == anyHlp );
	ASSERT( !(fFloat != anyHlp) );
	ASSERT( fFloat.IsEqual(anyHlp) );

	// here we use 14 significant digits
	fFloat = -24.490123456789;
	double testDouble = -24.490123456789;
	ASSERT_EQUAL_DELTA( testDouble, fFloat.AsDouble(0), 0.000000000001 );
	ASSERT_EQUAL( "-24.4901234567", fFloat.AsString().SubString(0, 14) );
	ASSERT_EQUAL( "-24.490123456789", fFloat.AsString() );
	ASSERT_EQUAL( -24L, fFloat.AsLong() );
	ASSERT_EQUAL( -24L, fFloat.AsLong(1234) );
	ASSERT( !fFloat.AsBool() );
	ASSERT( fFloat.AsBool(true) );
	ASSERT_EQUAL( 0, fFloat.AsBool() );
	ASSERT_EQUAL_DELTA( (double) - 24.4901, fFloat.AsDouble(), 0.0001 );
	ASSERT( fFloat.AsDouble(2.3) == (double) - 24.490123456789 );
	ASSERT( fFloat.AsDouble(2.3) != (double) - 24.4901 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	ASSERT( fFloat.AsIFAObject(&testObj) == &testObj );
	ASSERT( fFloat.AsIFAObject(&testObj) != &testObjDummy );
	ASSERT( fFloat.AsIFAObject() == NULL );

	ASSERT_EQUAL( 16, fFloat.AsString().Length() );
	ASSERT_EQUAL( "-24.490123456789", fFloat.AsString("Default") );

	ASSERT( !fFloat.Contains("testSlot") );
	ASSERT_LESS( fFloat.FindValue("testSlot"), 0L );

	// Does it survive?
	fFloat.Remove(-1L);
	fFloat.Remove(0L);
	fFloat.Remove(1L);
	ASSERT_LESS( fFloat.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( fFloat.IsDefined("testSlots"), false );
	ASSERT( fFloat.SlotName(0) == NULL );
	ASSERT( fFloat.At(-1L) == fFloat.At(0L) );
	ASSERT( (fFloat.At(0L)).AsLong(3L) == 3L );
	ASSERT( fFloat.At(1L) == NULL );
	ASSERT( fFloat[1L] == NULL );

	Anything anyTest = -24.51;
	ASSERT( anyTest.AsLong() == -24L );
}

void AnythingConstructorsTest::CharStarConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	char charStarTest[50] = { 0 };
	memcpy(charStarTest, "A String", strlen("A String"));

	Anything anyCharStar(charStarTest), anyHlp = fString;

	ASSERT( anyCharStar.GetType() == AnyCharPtrType );
	ASSERT_EQUAL( anyCharStar.IsNull(), false );
	ASSERT_EQUAL( anyCharStar.IsNull(), false );
	ASSERT( anyCharStar.IsNull() == 0 );
	ASSERT_EQUAL( anyCharStar.GetSize(), 1L );
	ASSERT( anyCharStar == anyHlp );
	ASSERT( !(anyCharStar != anyHlp) );
	ASSERT( anyCharStar.IsEqual(anyHlp) );

	ASSERT( String("A String") == anyCharStar.AsCharPtr() );
	ASSERT( String("A String") == anyCharStar.AsCharPtr("Default") );
	ASSERT_EQUAL( anyCharStar.AsCharPtr(0), "A String");
	ASSERT_EQUAL( anyCharStar.AsLong(), 0L );
	ASSERT_EQUAL( anyCharStar.AsLong(1234), 1234L );
	ASSERT_EQUAL ( 1234L, anyCharStar.AsLong(1234) );
	ASSERT_EQUAL( anyCharStar.AsBool(), false );
	ASSERT_EQUAL( anyCharStar.AsBool(true), true );
	ASSERT( anyCharStar.AsBool() == 0 );
	ASSERT( anyCharStar.AsDouble() == (double)0 );
	ASSERT( anyCharStar.AsDouble(2.3) == (double)2.3 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	ASSERT( anyCharStar.AsIFAObject(&testObj) == &testObj );
	ASSERT( anyCharStar.AsIFAObject(&testObj) != &testObjDummy );
	ASSERT( anyCharStar.AsIFAObject() == NULL );

	ASSERT( (anyCharStar.AsString()).Length() == (long)strlen("A String") );
	ASSERT( (anyCharStar.AsString()).Capacity() >= (anyCharStar.AsString()).Length() );
	ASSERT( anyCharStar.AsString() == "A String" );
	ASSERT( anyCharStar.AsString("Default") == "A String" );

	ASSERT_EQUAL( anyCharStar.Contains("testSlot"), false );
	ASSERT_LESS( anyCharStar.FindValue("testSlot"), 0L );

	// Does it survive?
	anyCharStar.Remove(-1L);
	anyCharStar.Remove(0L);
	anyCharStar.Remove(1L);
	ASSERT_LESS( anyCharStar.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( anyCharStar.IsDefined("testSlots"), false );
	ASSERT( anyCharStar.SlotName(0) == NULL );
	ASSERT( anyCharStar.At(-1L) == anyCharStar.At(0L) );
	ASSERT( (anyCharStar.At(0L)).AsLong() == 0 );
	ASSERT( anyCharStar.At(1L) == NULL );
	ASSERT_EQUAL( anyCharStar[0L].AsCharPtr(""), "" );
	ASSERT( anyCharStar[1L] == NULL );
}

void AnythingConstructorsTest::CharStarLongConstr0Test() {
	Anything anyStringLen("abcdefgh", 8L), anyHlp;

	anyHlp = anyStringLen;

	ASSERT( anyStringLen.GetType() == AnyCharPtrType );
	ASSERT_EQUAL( anyStringLen.IsNull(), false );
	ASSERT_EQUAL( anyStringLen.IsNull(), false );
	ASSERT( anyStringLen.IsNull() == 0 );
	ASSERT_EQUAL( anyStringLen.GetSize(), 1L );
	ASSERT( anyStringLen == anyHlp );
	ASSERT( !(anyStringLen != anyHlp) );
	ASSERT( anyStringLen.IsEqual(anyHlp) );

	ASSERT( String("abcdefgh") == anyStringLen.AsCharPtr() );
	ASSERT( String("abcdefgh") == anyStringLen.AsCharPtr("Default") );
	ASSERT_EQUAL( anyStringLen.AsCharPtr(0), "abcdefgh");

	ASSERT_EQUAL( anyStringLen.AsLong(), 0L );
	ASSERT_EQUAL( anyStringLen.AsLong(1234), 1234L );
	ASSERT_EQUAL( anyStringLen.AsBool(), false );
	ASSERT_EQUAL( anyStringLen.AsBool(true), true );
	ASSERT( anyStringLen.AsBool() == 0 );
	ASSERT( anyStringLen.AsDouble() == (double)0 );
	ASSERT( anyStringLen.AsDouble(2.3) == (double)2.3 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	ASSERT( anyStringLen.AsIFAObject(&testObj) == &testObj );
	ASSERT( anyStringLen.AsIFAObject(&testObj) != &testObjDummy );
	ASSERT( anyStringLen.AsIFAObject() == NULL );

	ASSERT( (anyStringLen.AsString()).Length() == (long)strlen("abcdefgh") );
	ASSERT( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	ASSERT( (anyStringLen.AsString()).Capacity() >= (anyStringLen.AsString(0)).Length() );
	ASSERT( anyStringLen.AsString() == "abcdefgh" );
	ASSERT( anyStringLen.AsString("Default") == "abcdefgh" );

	ASSERT_EQUAL( anyStringLen.Contains("testSlot"), false );
	ASSERT_LESS( anyStringLen.FindValue("testSlot"), 0L );

	// Does it survive?
	anyStringLen.Remove(-1L);
	anyStringLen.Remove(0L);
	anyStringLen.Remove(1L);
	ASSERT_LESS( anyStringLen.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( anyStringLen.IsDefined("testSlots"), false );
	ASSERT( anyStringLen.SlotName(0) == NULL );
	ASSERT( anyStringLen.At(-1L) == anyStringLen.At(0L) );
	ASSERT( (anyStringLen.At(0L)).AsLong() == 0 );
	ASSERT( anyStringLen.At(1L) == NULL );
	ASSERT_EQUAL( anyStringLen[0L].AsCharPtr(""), "");
	ASSERT( anyStringLen[1L] == NULL );
}

void AnythingConstructorsTest::CharStarLongConstr1Test() {
	Anything anyStringLen("abcdefgh", 4L), anyHlp;

	anyHlp = anyStringLen;

	ASSERT( anyStringLen.GetType() == AnyCharPtrType );
	ASSERT_EQUAL( anyStringLen.IsNull(), false );
	ASSERT_EQUAL( anyStringLen.IsNull(), false );
	ASSERT( anyStringLen.IsNull() == 0 );
	ASSERT_EQUAL( anyStringLen.GetSize(), 1L );
	ASSERT( anyStringLen == anyHlp );
	ASSERT( !(anyStringLen != anyHlp) );
	ASSERT( anyStringLen.IsEqual(anyHlp) );

	ASSERT( String("abcd") == anyStringLen.AsCharPtr() );
	ASSERT( String("abcd") == anyStringLen.AsCharPtr("Default") );
	ASSERT_EQUAL( anyStringLen.AsCharPtr(0), "abcd");

	ASSERT_EQUAL( anyStringLen.AsLong(), 0L );

	ASSERT_EQUAL( anyStringLen.AsLong(1234), 1234L );

	ASSERT_EQUAL( anyStringLen.AsBool(), false );
	ASSERT_EQUAL( anyStringLen.AsBool(true), true );
	ASSERT( anyStringLen.AsBool() == 0 );
	ASSERT( anyStringLen.AsDouble() == (double)0 );
	ASSERT( anyStringLen.AsDouble(2.3) == (double)2.3 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	ASSERT( anyStringLen.AsIFAObject(&testObj) == &testObj );
	ASSERT( anyStringLen.AsIFAObject(&testObj) != &testObjDummy );
	ASSERT( anyStringLen.AsIFAObject() == NULL );

	ASSERT( (anyStringLen.AsString()).Length() == (long)strlen("abcd") );
	ASSERT( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	ASSERT( (anyStringLen.AsString()).Capacity() >= (anyStringLen.AsString(0)).Length() );
	ASSERT( anyStringLen.AsString() == "abcd" );
	ASSERT( anyStringLen.AsString("Default") == "abcd" );

	ASSERT_EQUAL( anyStringLen.Contains("testSlot"), false );
	ASSERT_LESS( anyStringLen.FindValue("testSlot"), 0L );

	// Does it survive?
	anyStringLen.Remove(-1L);
	anyStringLen.Remove(0L);
	anyStringLen.Remove(1L);
	ASSERT_LESS( anyStringLen.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( anyStringLen.IsDefined("testSlots"), false );
	ASSERT( anyStringLen.SlotName(0) == NULL );
	ASSERT( anyStringLen.At(-1L) == anyStringLen.At(0L) );
	ASSERT( (anyStringLen.At(0L)).AsLong() == 0 );
	ASSERT( anyStringLen.At(1L) == NULL );
	ASSERT_EQUAL( anyStringLen[0L].AsCharPtr(""), "");
	ASSERT( anyStringLen[1L] == NULL );
}

void AnythingConstructorsTest::CharStarLongConstr2Test() {
	// Frage:  Ist der Inhalt gleich wegen Zufall oder wird festgestellt, dass strlen("abcdefgh") < 10 ist?
#define TSTSTR "abcdefgh"
	Anything anyStringLen(TSTSTR, 10L);
	// PT: this is not nice: constructor will read past the end of the string,
	//     it might crash!
	long l = anyStringLen.AsString().Length();

	ASSERT( l == (long)strlen(TSTSTR) ); // this would also be a reasonable assertion
	// ASSERT_EQUAL( 10, l );				// semantic change, above is correct!

	// ASSERT( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	// the above would be reasonable also
	ASSERT_EQUAL( anyStringLen.AsCharPtr(0), "abcdefgh");
}

void AnythingConstructorsTest::CharStarLongConstr3Test() {
	// negative Werte bedeuten "Ich kenne die Laenge nicht" --> Die Laenge wird die Laenge der Zeichenkette

	Anything anyStringLen("abcdefgh", -3L);

	ASSERT( (anyStringLen.AsString()).Length() == (long)strlen("abcdefgh") );
	ASSERT( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	ASSERT( anyStringLen.AsString() == "abcdefgh" );

	ASSERT( (anyStringLen.AsString()).Length() <= (anyStringLen.AsString()).Capacity() );
}

void AnythingConstructorsTest::StringConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	String stringTest = "A String";
	Anything anyString(stringTest), anyHlp = fString;

	ASSERT( anyString.GetType() == AnyCharPtrType );
	ASSERT_EQUAL( anyString.IsNull(), false );
	ASSERT_EQUAL( anyString.IsNull(), false );
	ASSERT( anyString.IsNull() == 0 );
	ASSERT_EQUAL( anyString.GetSize(), 1L );
	ASSERT( anyString.AsString() == anyHlp.AsString() );
	ASSERT( !(anyString.AsString() != anyHlp.AsString()) );
	ASSERT( anyString.IsEqual(anyHlp) );

	ASSERT_EQUAL( anyString.AsCharPtr(0), "A String");
	ASSERT_EQUAL( anyString.AsCharPtr("Default"), "A String");
	ASSERT( anyString.AsLong() == 0L );
	ASSERT_EQUAL( anyString.AsLong(1234), 1234L );

	ASSERT_EQUAL( anyString.AsBool(), false );
	ASSERT_EQUAL( anyString.AsBool(true), true );
	ASSERT( anyString.AsBool() == 0 );
	ASSERT( anyString.AsDouble() == (double)0 );
	ASSERT( anyString.AsDouble(2.3) == (double)2.3 );

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	ASSERT( anyString.AsIFAObject(&testObj) == &testObj );
	ASSERT( anyString.AsIFAObject(&testObj) != &testObjDummy );
	ASSERT( anyString.AsIFAObject() == NULL );

	ASSERT( (anyString.AsString()).Length() == (long)strlen("A String") );
	ASSERT( (anyString.AsString()).Capacity() >= (anyString.AsString()).Length() );
	ASSERT( anyString.AsString() == "A String" );
	ASSERT( anyString.AsString("Default") == "A String" );

	String stringHlp;
	stringHlp = anyString.AsString();
	ASSERT( stringHlp == "A String" );
	stringHlp = String(anyString.AsString());
	ASSERT( stringHlp == "A String" );

	ASSERT_EQUAL( anyString.Contains("testSlot"), false );
	ASSERT_LESS( anyString.FindValue("testSlot"), 0L );

	// Does it survive?
	anyString.Remove(-1L);
	anyString.Remove(0L);
	anyString.Remove(1L);
	ASSERT_LESS( anyString.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( anyString.IsDefined("testSlots"), false );
	ASSERT( anyString.SlotName(0) == NULL );
	ASSERT( anyString.At(-1L) == anyString.At(0L) );
	ASSERT( (anyString.At(0L)).AsLong() == 0L );
	ASSERT( anyString.At(1L) == NULL );
	ASSERT_EQUAL( anyString[0L].AsCharPtr(""), "");
	ASSERT( anyString[1L] == NULL );

	String voidstr((void *) "abc\0ef", 5); // string with 0 byte included
	Anything avoidstr(voidstr);
	ASSERT(avoidstr.GetType() == AnyCharPtrType);
	ASSERT(avoidstr.AsString() == voidstr); // does it remain the same
	ASSERT_EQUAL(voidstr.Length(), avoidstr.AsString().Length());
}

void AnythingConstructorsTest::EmptyVoidStarLenConstrTest() {
	char test[10];
	memset(test, '\0', 10);
	Anything anyTest((void *) 0, 10);

	ASSERT( anyTest.GetType() == AnyVoidBufType );
	ASSERT_EQUAL( anyTest.IsNull(), false );
	ASSERT_EQUAL( anyTest.GetSize(), 1L );
	ASSERT( anyTest.AsCharPtr(0) != 0 );
	ASSERT_EQUAL( (const char *) test, anyTest.AsCharPtr());

	Anything anyTest1((void *) test, 0); // we do not allocate something
	ASSERT( anyTest1.GetType() == AnyVoidBufType );
	ASSERT_EQUAL( anyTest1.IsNull(), false );
	ASSERT_EQUAL( anyTest1.GetSize(), 1L );
	ASSERT( anyTest1.AsCharPtr(0) == 0 );

	test[0] = '1';
	test[5] = '6';
	Anything anyTest2((void *) test, 10);
	ASSERT( anyTest2.GetType() == AnyVoidBufType );
	ASSERT_EQUAL( anyTest2.IsNull(), false );
	ASSERT_EQUAL( anyTest2.GetSize(), 1L );
	ASSERT( anyTest2.AsCharPtr(0) != 0 );
	ASSERT_EQUAL( (const char *) test, anyTest2.AsCharPtr());
}

void AnythingConstructorsTest::VoidStarLenConstrTest() {
	typedef long arrValueType;
	arrValueType arrTest[5] = { 0, 1, 2, 3, 4 };
	Anything anyTest((void *) arrTest, static_cast<long>(sizeof(arrTest)));
	size_t _byteLengthOfArray = sizeof(arrTest);
	long _elementsInArray = _byteLengthOfArray/sizeof(arrValueType);
	Anything anyHlp = anyTest;

	ASSERT( anyTest.GetType() == AnyVoidBufType );
	ASSERT_EQUAL( anyTest.IsNull(), false );
	ASSERT_EQUAL( anyTest.IsNull(), false );
	ASSERT( anyTest.IsNull() == 0 );
	ASSERT_EQUAL( anyTest.GetSize(), 1L );
	ASSERT( anyTest == anyHlp );
	ASSERT( !(anyTest != anyHlp) );
	ASSERT( anyTest.IsEqual(anyHlp) );

	ASSERT_EQUAL( (const char *) arrTest, anyTest.AsCharPtr());
	// AsCharPtr returns the address of the buffer of the binary any
	ASSERT( anyTest.AsLong() != 0 ); // address of the buffer (also)
	ASSERT_EQUAL((long) anyTest.AsCharPtr(0), anyTest.AsLong());
	ASSERT_EQUAL( anyTest.AsBool(), false );
	ASSERT_EQUAL( anyTest.AsBool(false), false );
	ASSERT( anyTest.AsDouble() == (double)0 );
	ASSERT_EQUAL( anyTest.AsDouble(2.3), 2.3 );

	DummyIFAObj testObj("Test");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	ASSERT( anyTest.AsIFAObject(&testObj) == &testObj );
	ASSERT( anyTest.AsIFAObject() == NULL );

	ASSERT_EQUAL( (long)sizeof(arrTest), (anyTest.AsString()).Length() );
	ASSERT( (anyTest.AsString()).Capacity() >= (anyTest.AsString()).Length() );
	ASSERT( anyTest.AsString() == String( (void *)arrTest, (long)sizeof(arrTest) ) );
	ASSERT( anyTest.AsString("Default") == String( (void *)arrTest, (long)sizeof(arrTest) ) );

	long i;
	for (i = 0; i < _elementsInArray; i++) {
		ASSERT ( ( (long *)((const char *)anyTest.AsString()) )[i] == i );
		ASSERT ( ( (long *)((const char *)anyTest.At(0L).AsString()) )[i] == i );
		ASSERT ( ( (long *)((const char *)anyTest[0L].AsString()) )[i] == i );

		ASSERT ( ( (long *)(anyTest.AsCharPtr()) )[i] == i );
		ASSERT ( ( (long *)(anyTest.At(0L).AsCharPtr()) )[i] == i );
		ASSERT ( ( (long *)((const char *)anyTest[0L].AsString()) )[i] == i );
	}

	ASSERT_EQUAL( anyTest.Contains("testSlot"), false );
	ASSERT_LESS( anyTest.FindValue("testSlot"), 0L );

	// Does it survive?
	ASSERT_EQUAL(_byteLengthOfArray, anyTest[0L].AsString().Length());
	ASSERT_EQUAL("", anyTest[0L].AsString());
	anyTest.Remove(-1L);
	ASSERT_EQUAL(1, anyTest.GetSize());
	anyTest.Remove(0L);
	ASSERT_EQUAL(0, anyTest.GetSize());
	anyTest.Remove(1L);
	ASSERT_EQUAL(0, anyTest.GetSize());
	ASSERT_LESS( anyTest.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( anyTest.IsDefined("testSlots"), false );
	ASSERT( anyTest.SlotName(0) == NULL );
	ASSERT( anyTest.At(-1L) == anyTest.At(0L) );
	ASSERT_EQUAL( (anyTest.At(0L)).AsLong(-1L), -1L );
	ASSERT( anyTest.At(1L) == NULL );
	ASSERT_EQUAL( anyTest[0L].AsCharPtr(""), (const char *)arrTest);
	ASSERT( anyTest[1L] == NULL );
}

void AnythingConstructorsTest::IFAObjectStarConstrTest() {
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	String testAdr;
	testAdr << (long) &testObj; // the address of the object for comparison

	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	Anything anyIFAObj(&testObj), anyHlp = anyIFAObj;

	ASSERT( anyIFAObj.GetType() == AnyObjectType );
	ASSERT_EQUAL( anyIFAObj.IsNull(), false );
	ASSERT_EQUAL( anyIFAObj.IsNull(), false );
	ASSERT( anyIFAObj.IsNull() == 0 );
	ASSERT_EQUAL( anyIFAObj.GetSize(), 1L );
	ASSERT( anyIFAObj == anyHlp );
	ASSERT( !(anyIFAObj != anyHlp) );
	ASSERT( anyIFAObj.IsEqual(anyHlp) );

	ASSERT( String("IFAObject") == anyIFAObj.AsCharPtr() ); // Ist es OK ????
	ASSERT( String("IFAObject") == anyIFAObj.AsCharPtr("Default") );
	ASSERT_EQUAL( anyIFAObj.AsCharPtr(0), "IFAObject");
	ASSERT( anyIFAObj.AsLong() != 0L ); // address of object
	ASSERT( anyIFAObj.AsLong(1234) != 1234L );
	ASSERT_EQUAL( anyIFAObj.AsBool(), false );
	ASSERT_EQUAL( anyIFAObj.AsBool(false), false );
	ASSERT( anyIFAObj.AsDouble() == (double)0 );
	ASSERT_EQUAL( anyIFAObj.AsDouble(2.3), 2.3 );

	ASSERT( anyIFAObj.AsIFAObject(&testObj) == &testObj );
	ASSERT( anyIFAObj.AsIFAObject(&testObj) != &testObjDummy );
	ASSERT( anyIFAObj.AsIFAObject() == &testObj );

	// PT: somewhat inconsistent, AsString returns the address, AsCharPtr the type
	// PS: now it is consistent so the testcases fail
	ASSERT( String("IFAObject") == anyIFAObj.AsString() ); // Ist es OK ????
	ASSERT( (anyIFAObj.AsString()).Capacity() >= (anyIFAObj.AsString()).Length() );

	ASSERT( (anyIFAObj.At(0L)).AsLong() == (long)&testObj );
	// returns the address
	ASSERT( anyIFAObj.At(1L) == NULL );

	ASSERT_EQUAL( anyIFAObj[0L].AsCharPtr(), "IFAObject");
	ASSERT_EQUAL( anyIFAObj.Contains("testSlot"), false );
	ASSERT_LESS( anyIFAObj.FindValue("testSlot"), 0L );

	// Does it survive?
	anyIFAObj.Remove(-1L);

	ASSERT_EQUAL( anyIFAObj.At(-1L).GetType(), AnyObjectType );

	ASSERT( (anyIFAObj.At(0L)).AsLong() == (long)&testObj );
	ASSERT( anyIFAObj.At(1L) == NULL );
	// PT: Probably this statement switches to an ArrayImpl
	ASSERT_EQUAL(anyIFAObj.GetSize(), 2 );
	ASSERT(anyIFAObj[0L].AsIFAObject() == &testObj );
	ASSERT( anyIFAObj[1L] == NULL );
	//ASSERT( anyIFAObj[0L].AsString() == testAdr );
	ASSERT_EQUAL( "IFAObject", anyIFAObj[0L].AsCharPtr() );

	anyIFAObj.Remove(1L);
	ASSERT_LESS( anyIFAObj.FindIndex("testSlots"), 0L );
	ASSERT_EQUAL( anyIFAObj.IsDefined("testSlots"), false );
	ASSERT( anyIFAObj.SlotName(0) == NULL );
	ASSERT( anyIFAObj[1L] == NULL );

	anyIFAObj.Remove(0L);
	ASSERT_EQUAL( (anyIFAObj.At(0L)).AsLong(), 0L );
	ASSERT_EQUAL( anyIFAObj[0L].GetType(), AnyNullType );
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

