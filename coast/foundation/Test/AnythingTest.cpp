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
#include "System.h"
#include "Dbg.h"
#include "AnyIterators.h"

//--- c-library modules used ---------------------------------------------------
#include <string.h>

//---- AnythingTest ---------------------------------------------------------

AnythingTest::AnythingTest (TString tname) : TestCaseType(tname)
{
}

void AnythingTest::setUp ()
{
	StartTrace(AnythingTest.setUp);
	fString = "A String";
	fLong = 5L;
	fBool = true;
	fDouble = 7.125;		// there is an exact binary representation
	fDouble2 = 8.1;			// there is no exact binary representation
	fFloat = -24.490123456789;
	fNull = Anything();
	fArray["slot"] = "contents";
	fArray["second"] = "more contents";
	fSequence.Append("A String");
	fSequence.Append("Another String");
	fSecondLevel["array"] = fArray;		// this is reference semantics
	fSecondLevel["sequence"] = fSequence.DeepClone();	// this is value semantics
	fThirdLevel["Level"] = fSecondLevel.DeepClone();
	fNullSlot["test"] = "some Text";
	fNullSlot[""] = "more Text";
	fSecondLevelNull["first"]["test"]["something"] = "more";
	fSecondLevelNull["first"][""] = "nothing";
	fSecondLevelNull["second"][""]["more"] = "after nothing";
	fSecondLevelNull["second"][""]["more"] = 2L;
	fSecondLevelNull["second"][""]["at"] = "least";
	if ( !System::LoadConfigFile(fConfig, "AnythingTest", "any") ) {
		assertEqual( "'read AnythingTest.any'", "'could not read AnythingTest.any'" );
	}
	fQuery = fConfig["Queries"][name()];
}

Test *AnythingTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AnythingTest, Constructors);
	ADD_CASE(testSuite, AnythingTest, KeyDeletion);
	ADD_CASE(testSuite, AnythingTest, EmptyAccess);
	ADD_CASE(testSuite, AnythingTest, KeyAccess);
	ADD_CASE(testSuite, AnythingTest, LookUpTest);
	ADD_CASE(testSuite, AnythingTest, TypeTest);
	ADD_CASE(testSuite, AnythingTest, RefSlotTest);
	ADD_CASE(testSuite, AnythingTest, AnyIncludeTest);
	ADD_CASE(testSuite, AnythingTest, SuccessiveAssignments);
	ADD_CASE(testSuite, AnythingTest, BinaryBufOutput);
	ADD_CASE(testSuite, AnythingTest, roConversion);
	ADD_CASE(testSuite, AnythingTest, operatorAssignemnt);
	ADD_CASE(testSuite, AnythingTest, appendTest);
	ADD_CASE(testSuite, AnythingTest, ReadFailsTest);
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
	ADD_CASE(testSuite, AnythingTest, ImportTest);

	ADD_CASE(testSuite, AnythingTest, WriteReadTest);
	ADD_CASE(testSuite, AnythingTest, DeepCloneWithRef);
	ADD_CASE(testSuite, AnythingTest, CharStarConstrTest);

	return testSuite;
}

void AnythingTest::ImportTest()
{
	istream *ifp = System::OpenStream("tmp/ImportTest", "any");
	if (t_assertm(ifp != 0, "expected ImportTest file to be there")) {
		Anything config;
		config.Import(*ifp);
		assertEqual(2, config.GetSize());
		Anything expectedValue;
		expectedValue["DLL"][0L] = "libsecurity.so";
		expectedValue.Append("grmbl");
		assertAnyEqual(expectedValue, config);
		delete ifp;
	}
}

/*============================================================================================================*/
/*  C L A S S M E T H O D S  (helpmethods)       Beginn                                                       */
/*============================================================================================================*/
Anything AnythingTest::init5DimArray(long anzElt)
{
	long		i0, i1;
	char		idx0[3] = {0}, idx1[3] = {0};
	Anything	anyInit;

	for ( i0 = '0'; i0 < anzElt + '0'; i0++ ) {
		long l0 = (long)(i0 - '0');
		idx0[0L] = (char)i0;

		for ( i1 = '0'; i1 < anzElt + '0'; i1++ ) {
			long l1 = (long)(i1 - '0');
			idx1[0L] = (char)i1;
			anyInit[idx0][idx1] = (l0) + (l1);
		}
	}
	return( anyInit );
}

bool AnythingTest::check5DimArray( Anything &any0, Anything &any1, long anzElt )
{
	long		i0, i1;
	char		idx0[3] = {0}, idx1[3] = {0};
	bool		retVal = true;
	Anything	any;

	for ( i0 = '0'; i0 < anzElt + '0'; i0++ ) {
		idx0[0L] = (char)i0;
		if ( any0[idx0].At("0").At("0").At("0").At("0") != any1[idx0].At("0").At("0").At("0").At("0") ) {
			retVal = false;
		}
		for ( i1 = '0'; i1 < anzElt + '0'; i1++ ) {
			idx1[0L] = (char)i1;
			if ( any0[idx0][idx1].At("0").At("0").At("0") != any1[idx0][idx1].At("0").At("0").At("0") ) {
				retVal = false;
			}
		}
	}
	return( retVal );
}

/*======================================================================================================================*/
/*  C L A S S M E T H O D S  (helpmethods)       Ende                                                                   */
/*======================================================================================================================*/

/*======================================================================================================================*/
/*  C O N S T R U C T O R S        Beginn                                                                               */
/*======================================================================================================================*/

class DummyIFAObj: public IFAObject
{
public:
	DummyIFAObj(const char *) { }
	IFAObject *Clone() const {
		return new DummyIFAObj("dummy");
	}
};

void AnythingTest::DefaultConstrTest()
{
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
	t_assert( fNull.AsString() == -1 );
	t_assert( fNull.AsString("Default") == "Default" );

	t_assert( fNull.Contains("testSlot") == false );
	assertCompare( fNull.FindValue("testSlot"), less, 0L );

	// Does it survive?
	fNull.Remove( -1L );
	fNull.Remove( 0L );
	fNull.Remove( 1L );
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

void AnythingTest::IntConstrTest()
{
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
	fBool.Remove( -1L );
	fBool.Remove( 0L );
	fBool.Remove( 1L );
	assertCompare( fBool.FindIndex("testSlots"), less, 0L );
	assertCompare( fBool.IsDefined("testSlots"), equal_to, false );
	t_assert( fBool.SlotName(0) == NULL );
	assertCompare( (fBool.At(0L)).AsBool(), equal_to, true );
	t_assert( fBool.At(1L) == NULL );
	t_assert( fBool[1L] == NULL );
}

void AnythingTest::LongConstrTest()
{
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	Anything anyHlp = fLong;

	t_assert( fLong.GetType() == AnyLongType );
	assertCompare( fLong.IsNull(), equal_to, false );
	assertCompare( fLong.IsNull(), equal_to, false );
	t_assert( fLong.IsNull() == 0 );
	assertCompare( fLong.GetSize(), equal_to, 1L );
	t_assert( fLong == anyHlp );
	t_assert( !(fLong != anyHlp) );
	t_assert( fLong.IsEqual(anyHlp) );

	t_assert( String("5") == fLong.AsCharPtr() );
	t_assert( String("5") == fLong.AsCharPtr("Default") );
	t_assert( memcmp( fLong.AsCharPtr(0), "5", 1 ) == 0 );
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
	fLong.Remove( -1L );
	fLong.Remove( 0L );
	fLong.Remove( 1L );
	assertCompare( fLong.FindIndex("testSlots"), less, 0L );
	assertCompare( fLong.IsDefined("testSlots"), equal_to, false );
	t_assert( fLong.SlotName(0) == NULL );
	// t_assert( fLong.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	assertCompare( (fLong.At(0L)).AsLong(), equal_to, 5L );
	t_assert( fLong.At(1L) == NULL );
	// t_assert( fLong[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	t_assert( memcmp( fLong[0L].AsCharPtr(), "5", 1 ) == 0 );
	t_assert( fLong[1L] == NULL );
}

void AnythingTest::DoubleConstr0Test()
{
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
	t_assert( memcmp( fDouble.AsCharPtr(0), "7.125", strlen("7.125") ) == 0 );
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
	fDouble.Remove( -1L );
	fDouble.Remove( 0L );
	fDouble.Remove( 1L );
	assertCompare( fDouble.FindIndex("testSlots"), less, 0L );
	assertCompare( fDouble.IsDefined("testSlots"), equal_to, false );
	t_assert( fDouble.SlotName(0) == NULL );
	// t_assert( fDouble.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	assertCompare( (fDouble.At(0L)).AsLong(), equal_to, 7L );
	t_assert( fDouble.At(1L) == NULL );
	// t_assert( fDouble[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	t_assert( memcmp( fDouble[0L].AsCharPtr(), "7.125", strlen("7.125") ) == 0 );
	t_assert( fDouble[1L] == NULL );
}

void AnythingTest::DoubleConstr1Test()
{
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
	fDouble2.Remove( -1L );
	fDouble2.Remove( 0L );
	fDouble2.Remove( 1L );
	assertCompare( fDouble2.FindIndex("testSlots"), less, 0L );
	assertCompare( fDouble2.IsDefined("testSlots"), equal_to, false );
	t_assert( fDouble2.SlotName(0) == NULL );
	// t_assert( fDouble2.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	assertCompare( (fDouble2.At(0L)).AsLong(), equal_to, 8L );
	t_assert( fDouble2.At(1L) == NULL );
	// t_assert( fDouble2[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	t_assert( fDouble2[1L] == NULL );
}

void AnythingTest::FloatConstrTest()
{
	// Test if the dafault constructor generates an anything-object with good reactions.
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
	fFloat.Remove( -1L );
	fFloat.Remove( 0L );
	fFloat.Remove( 1L );
	assertCompare( fFloat.FindIndex("testSlots"), less, 0L );
	assertCompare( fFloat.IsDefined("testSlots"), equal_to, false );
	t_assert( fFloat.SlotName(0) == NULL );
	// t_assert( fFloat.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	t_assert( (fFloat.At(0L)).AsLong() ==  -24 );
	t_assert( fFloat.At(1L) == NULL );
	// t_assert( fFloat[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	t_assert( fFloat[1L] == NULL );

	Anything anyTest = -24.51;
	t_assert( anyTest.AsLong() == -24L );
}

void AnythingTest::CharStarConstrTest()
{
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	char charStarTest[50] = {0};
	memcpy( charStarTest, "A String", strlen("A String") );

	Anything	anyCharStar( charStarTest ), anyHlp = fString;

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
	t_assert( memcmp( anyCharStar.AsCharPtr(0), "A String", strlen("A String") ) == 0 );
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

	t_assert( (anyCharStar.AsString()).Length() == (long)strlen("A String")  );
	t_assert( (anyCharStar.AsString()).Capacity() >= (anyCharStar.AsString()).Length() );
	t_assert( anyCharStar.AsString() == "A String" );
	t_assert( anyCharStar.AsString("Default") == "A String" );

	assertCompare( anyCharStar.Contains("testSlot"), equal_to, false );
	assertCompare( anyCharStar.FindValue("testSlot"), less, 0L );

	// Does it survive?
	anyCharStar.Remove( -1L );
	anyCharStar.Remove( 0L );
	anyCharStar.Remove( 1L );
	assertCompare( anyCharStar.FindIndex("testSlots"), less, 0L );
	assertCompare( anyCharStar.IsDefined("testSlots"), equal_to, false );
	t_assert( anyCharStar.SlotName(0) == NULL );
	// t_assert( anyCharStar.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	t_assert( (anyCharStar.At(0L)).AsLong() == 0 );
	t_assert( anyCharStar.At(1L) == NULL );
	// t_assert( anyCharStar[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	t_assert( memcmp( anyCharStar[0L].AsCharPtr(), "A String", strlen("A String") ) == 0 );
	t_assert( anyCharStar[1L] == NULL );
}

void AnythingTest::CharStarLongConstr0Test()
{
	Anything anyStringLen( "abcdefgh", 8L ), anyHlp;

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
	t_assert( memcmp( anyStringLen.AsCharPtr(0), "abcdefgh", strlen(anyStringLen.AsCharPtr(0)) ) == 0 );
	t_assert( memcmp( anyStringLen.AsCharPtr(0), "abcdefgh", strlen("abcedfgh") ) == 0 );

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

	t_assert( (anyStringLen.AsString()).Length() == (long)strlen("abcdefgh")  );
	t_assert( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	t_assert( (anyStringLen.AsString()).Capacity() >= (anyStringLen.AsString(0)).Length() );
	t_assert( anyStringLen.AsString() == "abcdefgh" );
	t_assert( anyStringLen.AsString("Default") == "abcdefgh" );

	assertCompare( anyStringLen.Contains("testSlot"), equal_to, false );
	assertCompare( anyStringLen.FindValue("testSlot"), less, 0L );

	// Does it survive?
	anyStringLen.Remove( -1L );
	anyStringLen.Remove( 0L );
	anyStringLen.Remove( 1L );
	assertCompare( anyStringLen.FindIndex("testSlots"), less, 0L );
	assertCompare( anyStringLen.IsDefined("testSlots"), equal_to, false );
	t_assert( anyStringLen.SlotName(0) == NULL );
	// t_assert( anyStringLen.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	t_assert( (anyStringLen.At(0L)).AsLong() == 0 );
	t_assert( anyStringLen.At(1L) == NULL );
	// t_assert( anyStringLen[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	t_assert( memcmp( anyStringLen[0L].AsCharPtr(), "abcdefgh", strlen( anyStringLen[0L].AsCharPtr() ) ) == 0 );
	t_assert( anyStringLen[1L] == NULL );
}

void AnythingTest::CharStarLongConstr1Test()
{
	Anything anyStringLen( "abcdefgh", 4L ), anyHlp;

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
	t_assert( memcmp( anyStringLen.AsCharPtr(0), "abcd", strlen(anyStringLen.AsCharPtr(0)) ) == 0 );

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

	t_assert( (anyStringLen.AsString()).Length() == (long)strlen("abcd")  );
	t_assert( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	t_assert( (anyStringLen.AsString()).Capacity() >= (anyStringLen.AsString(0)).Length() );
	t_assert( anyStringLen.AsString() == "abcd" );
	t_assert( anyStringLen.AsString("Default") == "abcd" );

	assertCompare( anyStringLen.Contains("testSlot"), equal_to, false );
	assertCompare( anyStringLen.FindValue("testSlot"), less, 0L );

	// Does it survive?
	anyStringLen.Remove( -1L );
	anyStringLen.Remove( 0L );
	anyStringLen.Remove( 1L );
	assertCompare( anyStringLen.FindIndex("testSlots"), less, 0L );
	assertCompare( anyStringLen.IsDefined("testSlots"), equal_to, false );
	t_assert( anyStringLen.SlotName(0) == NULL );
	// t_assert( anyStringLen.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	t_assert( (anyStringLen.At(0L)).AsLong() == 0 );
	t_assert( anyStringLen.At(1L) == NULL );
	// t_assert( anyStringLen[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	t_assert( memcmp( anyStringLen[0L].AsCharPtr(), "abcd", strlen( anyStringLen[0L].AsCharPtr() ) ) == 0 );
	t_assert( anyStringLen[1L] == NULL );
}

void AnythingTest::CharStarLongConstr2Test()
{
	// Frage:  Ist der Inhalt gleich wegen Zufall oder wird festgestellt, dass strlen("abcdefgh") < 10 ist?
#define TSTSTR "abcdefgh"
	Anything anyStringLen( TSTSTR, 10L );
	// PT: this is not nice: constructor will read past the end of the string,
	//     it might crash!
	long l = anyStringLen.AsString().Length();

	t_assert( l == (long)strlen(TSTSTR)  );		// this would also be a reasonable assertion
	// assertLongsEqual( 10, l );				// semantic change, above is correct!

	// t_assert( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	// the above would be reasonable also

	t_assert( memcmp( anyStringLen.AsCharPtr(0), "abcdefgh", strlen(anyStringLen.AsCharPtr(0)) ) == 0 );
	t_assert( memcmp( anyStringLen.AsCharPtr(0), "abcdefgh", strlen("abcedfgh") ) == 0 );
}

void AnythingTest::CharStarLongConstr3Test()
{
	// negative Werte bedeuten "Ich kenne die Laenge nicht" --> Die Laenge wird die Laenge der Zeichenkette

	Anything anyStringLen( "abcdefgh", -3L );

	t_assert( (anyStringLen.AsString()).Length() == (long)strlen("abcdefgh") );
	t_assert( (anyStringLen.AsString()).Length() == (long)strlen(anyStringLen.AsCharPtr(0)) );
	t_assert( anyStringLen.AsString() == "abcdefgh" );

	t_assert( (anyStringLen.AsString()).Length() <= (anyStringLen.AsString()).Capacity() );
}

void AnythingTest::StringConstrTest()
{
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	String		stringTest = "A String";
	Anything	anyString( stringTest ), anyHlp = fString;

	t_assert( anyString.GetType() == AnyCharPtrType );
	assertCompare( anyString.IsNull(), equal_to, false );
	assertCompare( anyString.IsNull(), equal_to, false );
	t_assert( anyString.IsNull() == 0 );
	assertCompare( anyString.GetSize(), equal_to, 1L );
	t_assert( anyString.AsString() == anyHlp.AsString() );
	t_assert( !(anyString.AsString() != anyHlp.AsString()) );
	t_assert( anyString.IsEqual(anyHlp) );

	t_assert( memcmp( anyString.AsCharPtr(0), "A String", strlen("A String") ) == 0 );
	t_assert( memcmp( anyString.AsCharPtr(), "A String", strlen("A String") ) == 0 );
	t_assert( memcmp( anyString.AsCharPtr("Default"), "A String", strlen("A String") ) == 0 );
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

	t_assert( (anyString.AsString()).Length() == (long)strlen("A String")  );
	t_assert( (anyString.AsString()).Capacity() >= (anyString.AsString()).Length() );
	t_assert( anyString.AsString() == "A String" );
	t_assert( anyString.AsString("Default") == "A String" );

	String stringHlp;
	stringHlp = anyString.AsString();
	t_assert( stringHlp == "A String" );
	stringHlp = String( anyString.AsString() );
	t_assert( stringHlp == "A String" );

	assertCompare( anyString.Contains("testSlot"), equal_to, false );
	assertCompare( anyString.FindValue("testSlot"), less, 0L );

	// Does it survive?
	anyString.Remove( -1L );
	anyString.Remove( 0L );
	anyString.Remove( 1L );
	assertCompare( anyString.FindIndex("testSlots"), less, 0L );
	assertCompare( anyString.IsDefined("testSlots"), equal_to, false );
	t_assert( anyString.SlotName(0) == NULL );
//	t_assert( anyString.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	t_assert( (anyString.At(0L)).AsLong() == 0L );
	t_assert( anyString.At(1L) == NULL );
//	t_assert( anyString[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	t_assert( memcmp( anyString[0L].AsCharPtr(), "A String", strlen("A String") ) == 0 );
	t_assert( anyString[1L] == NULL );

	String voidstr((void *)"abc\0ef", 5); // string with 0 byte included
	Anything avoidstr(voidstr);
	t_assert(avoidstr.GetType() == AnyCharPtrType);
	t_assert(avoidstr.AsString() == voidstr); // does it remain the same
	assertEqual(voidstr.Length(), avoidstr.AsString().Length());
}

void AnythingTest::EmptyVoidStarLenConstrTest()
{
	char test[10];
	memset(test, '\0', 10);
	Anything anyTest( (void *)0, 10);

	t_assert( anyTest.GetType() == AnyVoidBufType );
	assertCompare( anyTest.IsNull(), equal_to, false );
	assertCompare( anyTest.GetSize(), equal_to, 1L );
	t_assert( anyTest.AsCharPtr(0) != 0 );
	t_assert( memcmp( (const char *) test, anyTest.AsCharPtr(), sizeof(test) ) == 0 );

	Anything anyTest1( (void *)test, 0 ); // we do not allocate something
	t_assert( anyTest1.GetType() == AnyVoidBufType );
	assertCompare( anyTest1.IsNull(), equal_to, false );
	assertCompare( anyTest1.GetSize(), equal_to, 1L );
	t_assert( anyTest1.AsCharPtr(0) == 0 );

	test[0] = '1';
	test[5] = '6';
	Anything anyTest2( (void *)test, 10 );
	t_assert( anyTest2.GetType() == AnyVoidBufType );
	assertCompare( anyTest2.IsNull(), equal_to, false );
	assertCompare( anyTest2.GetSize(), equal_to, 1L );
	t_assert( anyTest2.AsCharPtr(0) != 0 );
	t_assert( memcmp( (const char *) test, anyTest2.AsCharPtr(), sizeof(test) ) == 0 );
}

void AnythingTest::VoidStarLenConstrTest()
{
	long		arrTest[5]	= { 0, 1, 2, 3, 4 };
	Anything	anyTest( (void *)&arrTest, (long)sizeof(arrTest) );
	Anything	anyHlp = anyTest;

	t_assert( anyTest.GetType() == AnyVoidBufType );
	assertCompare( anyTest.IsNull(), equal_to, false );
	assertCompare( anyTest.IsNull(), equal_to, false );
	t_assert( anyTest.IsNull() == 0 );
	assertCompare( anyTest.GetSize(), equal_to, 1L );
	t_assert( anyTest == anyHlp );
	t_assert( !(anyTest != anyHlp) );
	t_assert( anyTest.IsEqual(anyHlp) );

	t_assert( memcmp( (const char *) arrTest, anyTest.AsCharPtr(), sizeof(arrTest) ) == 0 );
	t_assert( memcmp( (const char *) arrTest, anyTest.AsCharPtr(0), sizeof(arrTest) ) == 0 );
	// AsCharPtr returns the address of the buffer of the binary any
	t_assert( anyTest.AsLong() != 0 );	// address of the buffer (also)
	assertEqual((long) anyTest.AsCharPtr(0), anyTest.AsLong());
	assertCompare( anyTest.AsBool(), equal_to, false );
	assertCompare( anyTest.AsBool(false), equal_to, false );
	t_assert( anyTest.AsDouble() == (double)0 );
	assertCompare( anyTest.AsDouble(2.3), equal_to, 2.3 );

	DummyIFAObj testObj("Test");
	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	t_assert( anyTest.AsIFAObject(&testObj) == &testObj );
	t_assert( anyTest.AsIFAObject() == NULL );

	assertEqual( (long)sizeof(arrTest), (anyTest.AsString()).Length()  );
	t_assert( (anyTest.AsString()).Capacity() >= (anyTest.AsString()).Length() );
	t_assert( anyTest.AsString() == String( (void *)arrTest, (long)sizeof(arrTest) ) );
	t_assert( anyTest.AsString("Default") == String( (void *)arrTest, (long)sizeof(arrTest) ) );

	long i;
	for ( i = 0; i < 5; i++ ) {
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
	assertEqual(20, anyTest[0L].AsString().Length());
	assertEqual("", anyTest[0L].AsString());
	anyTest.Remove( -1L );
	assertEqual(1, anyTest.GetSize());
	anyTest.Remove( 0L );
	assertEqual(1, anyTest.GetSize());
	anyTest.Remove( 1L );
	assertEqual(1, anyTest.GetSize());
	assertCompare( anyTest.FindIndex("testSlots"), less, 0L );
	assertCompare( anyTest.IsDefined("testSlots"), equal_to, false );
	t_assert( anyTest.SlotName(0) == NULL );
	// t_assert( anyTest.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	t_assert( (anyTest.At(0L)).AsLong() != 0 );
	t_assert( anyTest.At(1L) == NULL );
	// t_assert( anyTest[-1L] == NULL );			// ABORT ????	Anything.cpp:1358
	t_assert( memcmp( anyTest[0L].AsCharPtr(), (char *)arrTest, (long)sizeof(arrTest)) == 0 );
	t_assert( anyTest[1L] == NULL );
}

void AnythingTest::IFAObjectStarConstrTest()
{
	// Test if the dafault constructor generates an anything-object with good reactions.
	// The method called now are assumed to work correctly (they are being tested later on).

	DummyIFAObj testObj("Test"), testObjDummy("TestObjDummy");
	String testAdr;
	testAdr << (long)&testObj;		// the address of the object for comparison

	// We use a DummyIFAObj, it is the simples IFAObject that can be instantiated
	Anything anyIFAObj( &testObj ), anyHlp = anyIFAObj;

	t_assert( anyIFAObj.GetType() == AnyObjectType );
	assertCompare( anyIFAObj.IsNull(), equal_to, false );
	assertCompare( anyIFAObj.IsNull(), equal_to, false );
	t_assert( anyIFAObj.IsNull() == 0 );
	assertCompare( anyIFAObj.GetSize(), equal_to, 1L );
	t_assert( anyIFAObj == anyHlp );
	t_assert( !(anyIFAObj != anyHlp) );
	t_assert( anyIFAObj.IsEqual(anyHlp) );

	t_assert( String("IFAObject") == anyIFAObj.AsCharPtr() );				// Ist es OK ????
	t_assert( String("IFAObject") == anyIFAObj.AsCharPtr("Default") );
	t_assert( memcmp( anyIFAObj.AsCharPtr(0), "IFAObject", strlen("IFAObject") ) == 0 );
	t_assert( anyIFAObj.AsLong() != 0L );			// address of object
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
	t_assert( String("IFAObject") == anyIFAObj.AsString() );				// Ist es OK ????
	t_assert( (anyIFAObj.AsString()).Capacity() >= (anyIFAObj.AsString()).Length() );

	t_assert( (anyIFAObj.At(0L)).AsLong() == (long)&testObj );
	// returns the address
	t_assert( anyIFAObj.At(1L) == NULL );

	t_assert( memcmp( anyIFAObj[0L].AsCharPtr(), "IFAObject", strlen("IFAObject") ) == 0 );
	t_assert( memcmp( (const char *)anyIFAObj[0L].AsCharPtr(), "IFAObject", strlen("IFAObject") ) == 0 );
	assertCharPtrEqual( anyIFAObj[0L].AsCharPtr(), "IFAObject" );

	assertCompare( anyIFAObj.Contains("testSlot"), equal_to, false );
	assertCompare( anyIFAObj.FindValue("testSlot"), less, 0L );

	// Does it survive?
	// anyIFAObj.Remove( -1L );						// ABORT ????

	// t_assert( anyIFAObj.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	t_assert( (anyIFAObj.At(0L)).AsLong() == (long)&testObj );
	t_assert( anyIFAObj.At(1L) == NULL );
	// PT: Probably this statement switches to an ArrayImpl
	assertLongsEqual(anyIFAObj.GetSize(), 2 );
	t_assert(anyIFAObj[0L].AsIFAObject() == &testObj );
	t_assert( anyIFAObj[1L] == NULL );
	//t_assert( anyIFAObj[0L].AsString() == testAdr );
	assertCharPtrEqual( "IFAObject", anyIFAObj[0L].AsCharPtr() );

	// t_assert( anyIFAObj[-1L] == NULL );			// ABORT ????	Anything.cpp:1358

	anyIFAObj.Remove( 1L );
	assertCompare( anyIFAObj.FindIndex("testSlots"), less, 0L );
	assertCompare( anyIFAObj.IsDefined("testSlots"), equal_to, false );
	t_assert( anyIFAObj.SlotName(0) == NULL );
	t_assert( anyIFAObj[1L] == NULL );

	// t_assert( anyIFAObj.At(-1L) == NULL );		// ABORT ????	Anything.cpp:1358
	anyIFAObj.Remove( 0L );
	assertCompare( (anyIFAObj.At(0L)).AsLong(), equal_to, 0L );
	t_assert( anyIFAObj[0L].GetType() == AnyNullType );
}

void AnythingTest::AnythingConstrTest()
{
	Anything any0;
	Anything anyTest0( any0 );
	anyTest0.IsEqual( any0 );
	any0.IsEqual( anyTest0 );

	Anything any1( 0L );
	Anything anyTest1( any1 );
	anyTest1.IsEqual( any1 );
	any1.IsEqual( anyTest1 );

	Anything any2( true );
	Anything anyTest2( any2 );
	anyTest2.IsEqual( any2 );
	any2.IsEqual( anyTest2 );

	Anything any3( (long)2);
	Anything anyTest3( any3 );
	anyTest3.IsEqual( any3 );
	any3.IsEqual( anyTest3 );

	Anything any4( (float)4.1 );
	Anything anyTest4( any4 );
	anyTest4.IsEqual( any4 );
	any4.IsEqual( anyTest4 );

	Anything any5( (double)5.2 );
	Anything anyTest5( any5 );
	anyTest5.IsEqual( any5 );
	any5.IsEqual( anyTest5 );

	Anything any6( "0123456789" );
	Anything anyTest6( any6 );
	anyTest6.IsEqual( any6 );
	any6.IsEqual( anyTest6 );

	Anything any7( "abcdefgh", 8);
	Anything anyTest7( any7 );
	anyTest7.IsEqual( any7 );
	any7.IsEqual( anyTest7 );

	Anything any8( String("Anything Test") );
	Anything anyTest8( any8 );
	anyTest8.IsEqual( any8 );
	any8.IsEqual( anyTest8 );

	long buf[5] = {0, 1, 2, 3, 4};
	Anything any9( (void *)&buf[0], (long)sizeof(buf) );
	Anything anyTest9( any9 );
	anyTest9.IsEqual( any9 );
	any9.IsEqual( anyTest9 );

	DummyIFAObj testObj("Test");
	// We use a DummyIFAObj, it is the simplest IFAObject that can be instantiated
	Anything any10( &testObj );
	Anything anyTest10( any10 );
	anyTest10.IsEqual( any10 );
	any10.IsEqual( anyTest10 );
}

void AnythingTest::Constructors ()
/* what: test Anything constructors */
{
	t_assert(fNull == Anything());

	DefaultConstrTest();
	IntConstrTest();
	LongConstrTest();
	FloatConstrTest();
	DoubleConstr0Test();
	DoubleConstr1Test();
	CharStarConstrTest();
	CharStarLongConstr0Test();
	CharStarLongConstr1Test();
	CharStarLongConstr2Test();
	CharStarLongConstr3Test();
	StringConstrTest();
	VoidStarLenConstrTest();
	EmptyVoidStarLenConstrTest();
	IFAObjectStarConstrTest();
	AnythingConstrTest();

} // Constructors

/*==============================================================================================================*/
/*  C O N S T R U C T O R S        Ende                                                                         */
/*==============================================================================================================*/

/*==============================================================================================================*/
/*  D E L E T E - O P E R A T I O N S        Beginn                                                             */
/*==============================================================================================================*/

void AnythingTest::RemoveInvKeys ()
{
	long		i0, i1, i2;
	Anything	any0;

	// Prepare:  Fill an anything
	any0 = AnythingTest::init5DimArray(2);

	long	sizeBefore = any0.GetSize();
	char	buf[4] = {0};

	// Proper deletion test:  remove invalid SlotNames
	for ( i0 = 'a'; i0 < 'z'; i0++ ) {
		buf[0L] = (char)i0;
		for ( i1 = 'A'; i1 < 'Z'; i1++ ) {
			buf[1L] = (char)i1;
			for ( i2 = '0'; i2 < '9'; i2++ ) {
				buf[2L] = (char)i2;
				any0.Remove( buf );
				assertCompare( sizeBefore, equal_to, any0.GetSize() );
				t_assert( any0.IsDefined( buf ) == false );
			}
		}
	}

	// Remove indexes which do not exist
	for ( i0 = 1000000; i0 < 1002000; i0 += 137 ) {
		any0.Remove( i0 );
		assertCompare( sizeBefore, equal_to, any0.GetSize() );
//		t_assert( any0.IsDefined( i0 ) == false );
	}

	// Remove negative indexes
	for ( i0 = -1000L; i0 < 0L; i0 += 13 ) {
		any0.Remove( i0 );			// alert, but no crash
		assertCompare( sizeBefore, equal_to, any0.GetSize() );
//		t_assert( any0.IsDefined( i0 ) == false );
	}

	// Check if any0 till unchanged
	t_assert( AnythingTest::check5DimArray(any0, any0, 2) == true );
}

void AnythingTest::SimpleRemove ()
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

void AnythingTest::KeyDeletion ()
{
	SimpleRemove();
	RemoveInvKeys();
}

/*==============================================================================================================*/
/*  D E L E T E - O P E R A T I O N S        Ende                                                               */
/*==============================================================================================================*/

/*==============================================================================================================*/
/*  I N D E X - O P E R A T I O N S        Beginn                                                               */
/*==============================================================================================================*/
void AnythingTest::IndexAccess ()
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

void AnythingTest::KeyAccess0 ()
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

void AnythingTest::KeyAccess1 ()
{
	long		i0, i1, i2, i3, i4;
	char		idx0[3] = {0}, idx1[3] = {0}, idx2[3] = {0}, idx3[3] = {0}, idx4[3] = {0};
	Anything	any0;

	t_assert ( any0.GetSize() == 0 );

	// It does work because idx0, idx1, idx2, idx3, idx4 cannot be empty strings
	//----------------------------------------------------------------------------
	for ( i0 = '0'; i0 < '5'; i0++ ) {
		long l0 = (long)(i0 - '0');
		idx0[0L] = (char)i0;
		// The element being defined is nor yet defined:  verify
		assertCompare( any0.IsDefined(idx0), equal_to, false );
		t_assert( any0.FindIndex(idx0) == -1 );
		assertCompare( any0.IsDefined(l0), equal_to, false );
		t_assert( any0.FindIndex(l0) == -1 );
		assertCompare( any0.Contains(idx0), equal_to, false );
		for ( i1 = '0'; i1 < '5'; i1++ ) {
			long l1 = (long)(i1 - '0');
			idx1[0L] = (char)i1;
			// The element being defined is nor yet defined:  verify
			assertCompare( any0[idx0].IsDefined(idx1), equal_to, false );
			t_assert( any0[idx0].FindIndex(idx1) == -1 );
			assertCompare( any0[idx0].IsDefined(l1), equal_to, false );
			t_assert( any0[idx0].FindIndex(l1) == -1 );
			assertCompare( any0[idx0].Contains(idx1), equal_to, false );
			for ( i2 = '0'; i2 < '5'; i2++ ) {
				long l2 = (long)(i2 - '0');
				idx2[0L] = (char)i2;
				// The element being defined is nor yet defined:  verify
				assertCompare( any0[idx0][idx1].IsDefined(idx2), equal_to, false );
				t_assert( any0[idx0][idx1].FindIndex(idx2) == -1 );
				assertCompare( any0[idx0][idx1].IsDefined(l2), equal_to, false );
				t_assert( any0[idx0][idx1].FindIndex(l2) == -1 );
				assertCompare( any0[idx0][idx1].Contains(idx2), equal_to, false );
				for ( i3 = '0'; i3 < '5'; i3++ ) {
					long l3 = (long)(i3 - '0');
					idx3[0L] = (char)i3;
					// The element being defined is nor yet defined:  verify
					assertCompare( any0[idx0][idx1][idx2].IsDefined(idx3), equal_to, false );
					t_assert( any0[idx0][idx1][idx2].FindIndex(idx3) == -1 );
					assertCompare( any0[idx0][idx1][idx2].IsDefined(l3), equal_to, false );
					t_assert( any0[idx0][idx1][idx2].FindIndex(l3) == -1 );
					assertCompare( any0[idx0][idx1][idx2].Contains(idx3), equal_to, false );
					for ( i4 = '0'; i4 < '5'; i4++ ) {
						long l4 = (long)(i4 - '0');
						idx4[0L] = (char)i4;
						// The element being defined is nor yet defined:  verify
						assertCompare( any0[idx0][idx1][idx2][idx3].IsDefined(idx4), equal_to, false );
						t_assert( any0[idx0][idx1][idx2][idx3].FindIndex(idx4) == -1 );
						assertCompare( any0[idx0][idx1][idx2][idx3].IsDefined(l4), equal_to, false );
						t_assert( any0[idx0][idx1][idx2][idx3].FindIndex(l4) == -1 );
						any0[idx0][idx1][idx2][idx3][idx4] = l0 + l1 + l2 + l3 + l4;
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

	for ( i0 = '0'; i0 < '5'; i0++ ) {
		long l0 = (long)(i0 - '0');
		idx0[0L] = (char)i0;
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
		for ( i1 = '0'; i1 < '5'; i1++ ) {
			long l1 = (long)(i1 - '0');
			idx1[0L] = (char)i1;
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
			for ( i2 = '0'; i2 < '5'; i2++ ) {
				long l2 = (long)(i2 - '0');
				idx2[0L] = (char)i2;
				// The elements are now defined:  verify
				assertCompare( any0[idx0][idx1].IsDefined(idx2), equal_to, true );
				assertCompare( any0[idx0][idx1].FindIndex(idx2), equal_to, l2 );
				//t_assert( any0[idx0][idx1].Contains(idx2) == false );
				// Check the slot-names
				t_assert( strcmp( any0[idx0][idx1].SlotName(l2), idx2 ) == 0 );
				// Check 'AT'
				t_assert( any0[idx0][idx1].At(idx2).At("0").At("0") == (l0) + (l1) + (l2)  );
				t_assert( any0[idx0][idx1].At(l2).At(0L).At(0L) == (l0) + (l1) + (l2)  );
				for ( i3 = '0'; i3 < '5'; i3++ ) {
					long l3 = (long)(i3 - '0');
					idx3[0L] = (char)i3;
					// The elements are now defined:  verify
					t_assert( any0[idx0][idx1][idx2].IsDefined(idx3) == true );
					t_assert( any0[idx0][idx1][idx2].FindIndex(idx3) == l3 );
					//t_assert( any0[idx0][idx1][idx2].Contains(idx3) == false );
					// Check the slot-names
					t_assert( strcmp( any0[idx0][idx1][idx2].SlotName(l3), idx3 ) == 0 );
					// Check 'AT'
					t_assert( any0[idx0][idx1][idx2].At(idx3).At("0") == (l0) + (l1) + (l2) + (l3) );
					t_assert( any0[idx0][idx1][idx2].At(l3).At(0L) == (l0) + (l1) + (l2) + (l3) );
					for ( i4 = '0'; i4 < '5'; i4++ ) {
						long l4 = (long)(i4 - '0');
						idx4[0L] = (char)i4;
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

	t_assert ( any0.GetSize() == 6 );						// The first dimension has been incremented.  The other ones have not. WHY???
	t_assert ( any0[0L].GetSize() == 5 );
	t_assert ( any0[0L][0L].GetSize() == 5 );
	t_assert ( any0[0L][0L][0L].GetSize() == 5 );
	t_assert ( any0[0L][0L][0L][0L].GetSize() == 5 );
}
// KeyAccess1

void AnythingTest::KeyAccess2 ()
{
	// Anything used as a list
	Anything	array;
	long		i;

	for ( i = 0; i < 1000; i++ ) {
		if ( i < 300 ) {
			array.Append( i );
		} else if ( i < 500 ) {
			array.Append( (float)i + 0.12);
		} else if ( i < 800 ) {
			array.Append( -1.0 * (float)i - 0.12);
		} else {
			array.Append( "Test" );
		}
	}

	for ( i = 0; i < 1000; i++ ) {
		if ( i < 300 ) {
			t_assert( array[i] == i );
			t_assert( array[i].GetType() == AnyLongType );
		} else if ( i < 500 ) {
			t_assert( array[i] == (float)i + 0.12 );
			t_assert( array[i].GetType() == AnyDoubleType );
		} else if ( i < 800 ) {
			t_assert( array[i] == -(float)i - 0.12 );
			t_assert( array[i].GetType() == AnyDoubleType );
		} else {
			t_assert( array[i] == "Test" );
			t_assert( array[i].GetType() == AnyCharPtrType );
		}
	}
}
// KeyAccess2

void AnythingTest::MixKeysAndArray ()
{

	Anything any0, any_save, any_i0, any_i1;

	// Prepare:  Fill an anything
	any0 = AnythingTest::init5DimArray(2);
	any_save = any0;

	any_i0 = any0[0L];
	any_i1 = any0[1];

	t_assert( any0.GetSize() == 2 );
	any0.Append(233L);
	t_assert( any0.GetSize() == 3 );
	t_assert( any0[2].AsLong() == 233 );
	any0.Remove( 0L );
	t_assert( any0.GetSize() == 2 );
	t_assert( any0[1].AsLong() == 233 );
	any0.Remove( 0L );
	t_assert( any0.GetSize() == 1 );
	t_assert( any0[0L].AsLong() == 233 );
	any0.Remove( 0L );
	t_assert( any0.GetSize() == 0 );

	any0["0"] = 12345L;
	any0["1"] = any_i1;
	any0["0"] = any_i0;

	t_assert( AnythingTest::check5DimArray( any0, any_save, 2 ) == true );
}

void AnythingTest::Recursive()
{
	Anything any0;

	any0.Append( (long)12 );
	any0.Append( (long)23 );
	any0.Append( (long)45 );
	any0.Append( any0 );	// dangerous: may lead to a leak

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

	any0.Append( (long)67 );
	t_assert( any0[3][3][3][3][3][3][4].AsLong() == 67 );

	any0.Remove(3L);	// break circular dependency to avoid leak

	Anything any1;

	any1["0"] = (long)12;
	any1["1"] = (long)23;
	any1["2"] = (long)45;
	any1["3"] = any1;	// dangerous: may lead to a leak

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

	any1["4"] = (long)67;
	t_assert( any1["3"]["3"]["3"]["3"]["3"]["3"]["4"].AsLong() == 67 );

	any1.Remove("3");	// break circular dependency to avoid leak
}

void AnythingTest::KeyAccess ()
/* what: test access to Anythings through regular string keys
*/
{
	IndexAccess();
	KeyAccess0();
	KeyAccess1();
	KeyAccess2();
	MixKeysAndArray();
	Recursive();
}
/*==============================================================================================================*/
/*  I N D E X - O P E R A T I O N S        Ende                                                                 */
/*==============================================================================================================*/

/*==============================================================================================================*/
/*  I N D E X - O P E R A T I O N S  (with empty slotnames)              Beginn                                 */
/*==============================================================================================================*/
void AnythingTest::EmptyAccess0 ()
/* what: test access to Anythings with empty slots
		 As a matter of fact the result of access with empty slots is
		 undefined. The following tests just describes the behavior of
		 the current implementation.
		 (The behavior may seem weird in some places)
*/
{
	Anything test;

	int lengthBefore = test.GetSize();
	test[""] = 1L;		// empty slotname, added anonymously
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

void AnythingTest::EmptyAccess1 ()
/* what: test access to Anythings with empty slots
		 As a matter of fact the result of access with empty slots is
		 undefined. The following tests just describes the behavior of
		 the current implementation.
*/
{
	// the following tests only make sense if Coast is not compiled
	// with DEBUG enabled, since most of them are caught by an assert.

	Anything	test;
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

void AnythingTest::EmptyAccess ()
{
	EmptyAccess0 ();
	EmptyAccess1 ();
}

/*==============================================================================================================*/
/*  I N D E X - O P E R A T I O N S  (with empty slotnames)              Ende                                   */
/*==============================================================================================================*/

/*================================================================================================================*/
/*                       I N P U T / O U T P U T    T E S T S   begin                                             */
/*================================================================================================================*/
void AnythingTest::WriteRead0Test()
{
	Anything any0 = AnythingTest::init5DimArray(5);
	String buffer;
	{
		OStringStream os(&buffer);
		os << any0;
	}
	IStringStream is(buffer);
	Anything any1;
	is >> any1;

	t_assert( AnythingTest::check5DimArray( any0, any1, 5) == true );
}

void AnythingTest::WriteRead1Test()
{
	Anything any0 = AnythingTest::init5DimArray(5);

	ostream *os = System::OpenOStream("tmp/anything0", "tst", ios::out);
	if ( os ) {
		any0.PrintOn( *os, true );
		delete os;
	} else {
		assertEqual( "'write tmp/anything0.tst'", "'could not write tmp/anything0.tst'" );
	}

	Anything any1;
	istream *is = System::OpenStream("tmp/anything0", "tst");
	if ( is ) {
		any1.Import( *is );
		delete is;
	} else {
		assertEqual( "'read tmp/anything0.tst'", "'could not read tmp/anything0.tst'" );
	}

	t_assert( AnythingTest::check5DimArray( any0, any1, 5) == true );
}

void AnythingTest::WriteRead5Test()
{
	Anything	any0("Anything: test");

	ostream *os = System::OpenOStream("tmp/anything6", "tst", ios::out);
	if ( os ) {
		int i;
		for ( i = 0; i < 5; i++ ) {
			*os << any0;
		}
		delete os;
	} else {
		assertEqual( "'write tmp/anything6.tst'", "'could not write tmp/anything6.tst'" );
	}

	istream *is = System::OpenStream("tmp/anything6", "tst");
	if ( is ) {
		Anything any1;
		*is >> any1;
		delete is;
		t_assert( any0 == any1 );		// PATRU ????
		t_assert( any0.AsCharPtr() == any0.AsCharPtr() );
		t_assert( any0.AsString() == any0.AsString() );
		t_assert( any0.GetSize() == any1.GetSize() );
		t_assert( any0.GetType() == any1.GetType() );
		t_assert( any0.IsEqual(any1) );
	} else {
		assertEqual( "'read tmp/anything6.tst'", "'could not read tmp/anything6.tst'" );
	}
}

void AnythingTest::WriteRead7Test()
{
	Anything	any0("Anything: test");

	String buf;
	{
		OStringStream os(&buf);
		any0.PrintOn( os, false );
	}

	IStringStream is(buf);
	Anything any1;

	any1.Import( is );

	t_assert( any0 == any1 );		// equality works for simple StringImpls!
	assertEqual( any0.AsCharPtr(), any1.AsCharPtr() );
	t_assert( any0.AsString() == any1[0L].AsString() );
	assertEqual( any0.GetSize(), any1.GetSize() );
	t_assert( any0.GetType() == any1.GetType() );
	t_assert( any0.IsEqual(any1) );		// equality works for simple Strings
}

void AnythingTest::WriteRead8Test()
{
	Anything	any0, any1;

	any0 = AnythingTest::init5DimArray(5);

	ostream *os0 = System::OpenOStream("tmp/anything9", "tst", ios::out);
	if ( os0 ) {
		any0.Export( *os0, 0 );
		delete os0;
	} else {
		assertEqual( "'write tmp/anything9.tst'", "'could not write tmp/anything9.tst'" );
	}

	istream *is0 = System::OpenStream("tmp/anything9", "tst");
	if ( is0 ) {
		any1.Import( *is0 );
		delete is0;
	} else {
		assertEqual( "'read tmp/anything9.tst'", "'could not read tmp/anything9.tst'" );
	}

	t_assert( AnythingTest::check5DimArray(any0, any1, 5) == true );
}

void AnythingTest::TypeTest()
{
	Anything TypeChange;
	Anything ArrayTypeTest;
	Anything KeyTypeTest;
	Anything AnyLong( 5L );
	Anything AnyVoidBuff( (void *)"Bytebuffer", 10 );
	Anything SubAnything;
	String   MyString( "Another String" );

	// Simple Types
	t_assert( TypeChange.GetType() == AnyNullType );

	TypeChange = 12L;
	t_assert( TypeChange.GetType() == AnyLongType );

	TypeChange = (char *)"Ein String";
	t_assert( TypeChange.GetType() == AnyCharPtrType );

	TypeChange = (long)34;
	t_assert( TypeChange.GetType() == AnyLongType );

	TypeChange = (double)300.44;
	t_assert( TypeChange.GetType() == AnyDoubleType );

	TypeChange = (long)56;
	t_assert( TypeChange.GetType() == AnyLongType );

	TypeChange = MyString;
	t_assert( TypeChange.GetType() == AnyCharPtrType );

	TypeChange = (float)1.37;
	t_assert( TypeChange.GetType() == AnyDoubleType );

	TypeChange = AnyLong;
	t_assert( TypeChange.GetType() == AnyLongType );

	// Array
	ArrayTypeTest[""] = ( "Noch ein String" );
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[""] = ( 12L );
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );

	ArrayTypeTest[""] = ( (long)34 );
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 2].GetType() == AnyLongType );

	ArrayTypeTest[""] = ( (long)56 );
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 2].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 3].GetType() == AnyLongType );

	ArrayTypeTest[""] = ( (float)98.765 );
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 2].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 3].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 4].GetType() == AnyDoubleType );

	ArrayTypeTest[""] = ( (double)12.345 );
	t_assert( ArrayTypeTest.GetType() == AnyArrayType );
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );
	t_assert( ArrayTypeTest[ 1].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 2].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 3].GetType() == AnyLongType );
	t_assert( ArrayTypeTest[ 4].GetType() == AnyDoubleType );
	t_assert( ArrayTypeTest[ 5].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = 12L;
	t_assert( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = (char *)"Ein String";
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[0L] = 34L;
	t_assert( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = (double)300.44;
	t_assert( ArrayTypeTest[0L].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = (long)56;
	t_assert( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = (float)1.37;
	t_assert( ArrayTypeTest[0L].GetType() == AnyDoubleType );

	ArrayTypeTest[0L] = AnyLong;
	t_assert( ArrayTypeTest[0L].GetType() == AnyLongType );

	ArrayTypeTest[0L] = MyString;
	t_assert( ArrayTypeTest[0L].GetType() == AnyCharPtrType );

	SubAnything = ArrayTypeTest;
	ArrayTypeTest.Append( SubAnything );		// this will lead to a leak!
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

	ArrayTypeTest[6][0L] = (char *)"Ein String";
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[6][0L] = (long)34;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest[6][0L] = (double)300.44;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyDoubleType );

	ArrayTypeTest[6][0L] = (long)56;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest[6][0L] = MyString;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyCharPtrType );

	ArrayTypeTest[6][0L] = (float)1.37;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyDoubleType );

	ArrayTypeTest[6][0L] = AnyLong;
	t_assert( ArrayTypeTest[6][0L].GetType() == AnyLongType );

	ArrayTypeTest.Remove(6L);		// break circular dependency to avoid leak

	// Array
	KeyTypeTest["0"] = "Noch ein String";
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["1"] = ( 12L );
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );

	KeyTypeTest["2"] = ( (long)34 );
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["2"].GetType() == AnyLongType );

	KeyTypeTest["3"] = ( (long)56 );
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["2"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["3"].GetType() == AnyLongType );

	KeyTypeTest["4"] = ( (float)98.765 );
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["2"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["3"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["4"].GetType() == AnyDoubleType );

	KeyTypeTest["5"] = ( (double)12.345 );
	t_assert( KeyTypeTest.GetType() == AnyArrayType );
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );
	t_assert( KeyTypeTest["1"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["2"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["3"].GetType() == AnyLongType );
	t_assert( KeyTypeTest["4"].GetType() == AnyDoubleType );
	t_assert( KeyTypeTest["5"].GetType() == AnyDoubleType );

	KeyTypeTest["0"] = 12L;
	t_assert( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = (char *)"Ein String";
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["0"] = (long)34;
	t_assert( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = (double)300.44;
	t_assert( KeyTypeTest["0"].GetType() == AnyDoubleType );

	KeyTypeTest["0"] = (long)56;
	t_assert( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = (float)1.37;
	t_assert( KeyTypeTest["0"].GetType() == AnyDoubleType );

	KeyTypeTest["0"] = AnyLong;
	t_assert( KeyTypeTest["0"].GetType() == AnyLongType );

	KeyTypeTest["0"] = MyString;
	t_assert( KeyTypeTest["0"].GetType() == AnyCharPtrType );

	SubAnything = KeyTypeTest;
	KeyTypeTest["6"] = ( SubAnything );
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

	KeyTypeTest["6"]["0"] = (char *)"Ein String";
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["6"]["0"] = (long)34;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest["6"]["0"] = (double)300.44;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyDoubleType );

	KeyTypeTest["6"]["0"] = (long)56;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest["6"]["0"] = MyString;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyCharPtrType );

	KeyTypeTest["6"]["0"] = (float)1.37;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyDoubleType );

	KeyTypeTest["6"]["0"] = AnyLong;
	t_assert( KeyTypeTest["6"]["0"].GetType() == AnyLongType );

	KeyTypeTest.Remove("6");	// break circular dependency to avoid leak
}

void AnythingTest::RefSlotTest()
{
	{
		// Check Ref Export and Import of primitive types
		Anything a;

		Anything b;
		b[0L] = "hallo";
		b[1L] = "Peter";

		a["slot1"] = b;
		a["slot2"] = b;

		t_assert(((long)(a["slot1"].GetImpl()) != 0));
		assertEqual((long)(a["slot1"].GetImpl()), (long)(a["slot2"].GetImpl()));
		Anything c = a.DeepClone();
		assertEqual((long)(c["slot1"].GetImpl()), (long)(c["slot2"].GetImpl()));

		String tempString;

		OStringStream os(&tempString);

		a.Export(os);

		IStringStream is(tempString);

		c.Import(is);

		c["slot1"][0L] = "mod";
		assertEqual("mod", c["slot2"][0L].AsCharPtr("no"));
	}
	{
		// test a reference to a named anything
		String str(_QUOTE_( { /200 { /a gugus /b gaga } /100 %200 }));
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		IStringStream is(str);
		anyResult.Import(is);
		// test if the link was made
		anyExpected["100"] = anyExpected["200"];
		assertAnyEqual(anyExpected, anyResult);
		// try to modify the 'original' and compare
		anyResult["200"]["c"] = "fooBar";
		anyExpected["200"]["c"] = "fooBar";
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test the inverse order of the reference definition
		String str(_QUOTE_( { /1 %200 /200 { /a gugus /b gaga }}));
		IStringStream is(str);
		Anything anyResult, anyExpected;
		anyExpected["1"]["a"] = "gugus";
		anyExpected["1"]["b"] = "gaga";
		anyExpected["200"]    = anyExpected["1"];
		anyResult.Import(is);
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a reference to a named slot
		String str(_QUOTE_( { /200 { /a gugus /b gaga } /100 %200.a }));
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["100"] = anyExpected["200"]["a"];
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test unnamed references
		String str(_QUOTE_( { /200 { /a gugus /b gaga /c { %200.a }} /100 { c %200.b } %200.a }));
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected.Append(anyExpected["200"]["a"]);
		anyExpected["100"].Append(anyExpected["200"]["b"]);
		anyExpected["200"]["c"].Append(anyExpected["200"]["a"]);
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a double linked reference
		String str(_QUOTE_( { /200 { /a gugus /b gaga /c { %200.a }} /100 { c %200.b } %200.c }));
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["100"].Append(anyExpected["200"]["b"]);
		anyExpected["200"]["c"].Append(anyExpected["200"]["a"]);
		anyExpected.Append(anyExpected["200"]["c"]);
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a reference to a unnamed slot
		String str(_QUOTE_( { /200 { /a gugus /b gaga /c { 34 }} /100 { c %200.b /e %200.c:0 } %200.a }));
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected.Append(anyExpected["200"]["a"]);
		anyExpected["100"].Append(anyExpected["200"]["b"]);
		anyExpected["200"]["c"].Append(34L);
		anyExpected["100"]["e"] = anyExpected["200"]["c"][0L];
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a unnamed reference to a unnamed slot
		String str(_QUOTE_( { /200 { /a gugus /b gaga /c { 34 }} /100 { c %200.b /e %200.c:0 } %200.c:0 }));
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected["100"].Append(anyExpected["200"]["b"]);
		anyExpected["200"]["c"].Append(34L);
		anyExpected.Append(anyExpected["200"]["c"][0L]);
		anyExpected["100"]["e"] = anyExpected["200"]["c"][0L];
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a unnamed reference to a unnamed slot with a reference
		String str(_QUOTE_( { /200 { /a gugus /b gaga /c { %200.a }} /100 { c %200.b /e %200.c:0 } %200.a }));
		Anything anyResult, anyExpected;
		anyExpected["200"]["a"] = "gugus";
		anyExpected["200"]["b"] = "gaga";
		anyExpected["100"] = "c";
		IStringStream is(str);
		anyResult.Import(is);
		anyExpected.Append(anyExpected["200"]["a"]);
		anyExpected["100"].Append(anyExpected["200"]["b"]);
		anyExpected["200"]["c"].Append(anyExpected["200"]["a"]);
		anyExpected["100"]["e"] = anyExpected["200"]["c"][0L];
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a unnamed reference to a unnamed slot with a reference
		String str(_QUOTE_( { /100 { "a" { /200 "b" /300 %"100:1.200" }}}));
		Anything anyResult, anyExpected, temp1, temp2;

		temp1["200"] = "b";
		temp1["300"] = temp1["200"];
		temp2.Append("a");
		temp2.Append(temp1);
		anyExpected["100"] = temp2;

		IStringStream is(str);
		anyResult.Import(is);
		assertAnyEqual(anyExpected, anyResult);
	}
	{
		// test a unnamed and named reference where the last part of the key contains 'special characters'
		// like indexDelimiter or pathDelimiter!!
		/* here is the complete Anything, listed for better understanding
		   especially have a look at the slotnames "23:21:36" and "23.22.28"
		   until now this path had been resolved too and lead to unfindable slots... which is in fact a bug!
		{
		  /P {
			/T {
			  /T {
				{ a }
				{ b }
				{ c }
			  }
			}
		  }
		  /T {
			/"2002" {
			  /"03" {
				/"20" {
				  %"P.T.T:0"
				  /"23:21:36" %"P.T.T:1"
				  /"23.22.28" %"P.T.T:2"
				}
			  }
			}
		  }
		}
		*/
		String str(_QUOTE_( { /P { /T { /T {{ a }{ b }{ c }} }} /T { /"2002" { /"03" { /"20" { %"P.T.T:0" /"23:21:36" %"P.T.T:1" /"23.22.28" %"P.T.T:2"}}}}}));
		Anything anyResult, anyExpected, temp1, temp2;

		anyExpected["P"]["T"]["T"][0L][0L] = "a";
		anyExpected["P"]["T"]["T"][1L][0L] = "b";
		anyExpected["P"]["T"]["T"][2L][0L] = "c";
		anyExpected["T"]["2002"]["03"]["20"][0L] = anyExpected["P"]["T"]["T"][0L];
		anyExpected["T"]["2002"]["03"]["20"]["23:21:36"] = anyExpected["P"]["T"]["T"][1L];
		anyExpected["T"]["2002"]["03"]["20"]["23.22.28"] = anyExpected["P"]["T"]["T"][2L];

		IStringStream is(&str);
		anyResult.Import(is);
		assertAnyEqual(anyExpected, anyResult);
	}
}

void AnythingTest::AnyIncludeTest()
{
	{
		// Test an include in a unnamed slot with a relativ uri without localhost
		Anything anyMain, anyIncl, anyRef;

		String strMain(_QUOTE_( { /200 { /a gugus /b gaga } !"file:///include.any" }));
		String strIncl(_QUOTE_( { /100 { /d foo /e frim } }));
		String strRef (_QUOTE_( { /200 { /a gugus /b gaga } { /100 { /d foo /e frim } } }));
		{
			IStringStream is(strIncl);
			anyIncl.Import(is);
			iostream *pStream = System::OpenOStream("include", "any");
			if (pStream) {
				anyIncl.Export(*pStream);
				delete pStream;
			}
		}
		{
			IStringStream is(strMain);
			anyMain.Import(is);
		}
		{
			IStringStream is(strRef);
			anyRef.Import(is);
		}
		assertAnyEqual(anyRef, anyMain);
	}
	{
		// Test an include in a unnamed slot with a absolut uri without localhost
		Anything anyMain, anyIncl, anyRef;

		String strMain;
		strMain << "{ /200 { /a gugus /b gaga } !\"file:///" << System::GetFilePath("include", "any") << "\"}";
		String strIncl(_QUOTE_( { /100 { /d foo /e frim } }));
		String strRef (_QUOTE_( { /200 { /a gugus /b gaga } { /100 { /d foo /e frim } } }));
		{
			IStringStream is(strIncl);
			anyIncl.Import(is);
			iostream *pStream = System::OpenOStream("include", "any");
			if (pStream) {
				anyIncl.Export(*pStream);
				delete pStream;
			}
		}
		{
			IStringStream is(strMain);
			anyMain.Import(is);
		}
		{
			IStringStream is(strRef);
			anyRef.Import(is);
		}
		assertAnyEqual(anyRef, anyMain);
	}
//FIXME: needs further analysis what to do when specifying a machine name
// it seems that when a machine name other than localhost is used wh should use FTP to get the file
//	{
//		// Test an include in a named slot with a relativ uri with localhost
//		Anything anyMain, anyIncl, anyRef;
//
//		String strMain(_QUOTE_({ /200 { /a gugus /b gaga } /300 !"file://localhost/include.any" }));
//		String strIncl(_QUOTE_({ /100 { /d foo /e frim } }));
//		String strRef (_QUOTE_({ /200 { /a gugus /b gaga } /300 { /100 { /d foo /e frim } } }));
//		{
//			IStringStream is(strIncl);
//			anyIncl.Import(is);
//			iostream *pStream = System::OpenOStream("include", "any");
//			if (pStream)
//			{
//				anyIncl.Export(*pStream);
//				delete pStream;
//			}
//		}
//		{
//			IStringStream is(strMain);
//			anyMain.Import(is);
//		}
//		{
//			IStringStream is(strRef);
//			anyRef.Import(is);
//		}
//		assertAnyEqual(anyRef, anyMain);
//	}
//	{
//		// Test an include with missing of the file
//		Anything anyMain, anyIncl, anyRef;
//
//		String strMain(_QUOTE_({ /200 { /a gugus /b gaga } /300 !"file://localhost/gugus.any" }));
//		String strIncl(_QUOTE_({ /100 { /d foo /e frim } }));
//		String strRef (_QUOTE_({ /200 { /a gugus /b gaga } /300  * }));
//		{
//			IStringStream is(strIncl);
//			anyIncl.Import(is);
//			iostream *pStream = System::OpenOStream("include", "any");
//			if (pStream)
//			{
//				anyIncl.Export(*pStream);
//				delete pStream;
//			}
//		}
//		{
//			IStringStream is(strMain);
//			anyMain.Import(is);
//		}
//		{
//			IStringStream is(strRef);
//			anyRef.Import(is);
//		}
//		assertAnyEqual(anyRef, anyMain);
//	}
	{
		// Test an include with a query for a subtree in the include file
		Anything anyMain, anyIncl, anyRef;

		String strMain(_QUOTE_( { /200 { /a gugus /b gaga } !"file:///include.any?100.d:0" }));
		String strIncl(_QUOTE_( { /100 { /d { foo } /e frim } }));
		String strRef (_QUOTE_( { /200 { /a gugus /b gaga } "foo" }));
		{
			IStringStream is(strIncl);
			anyIncl.Import(is);
			iostream *pStream = System::OpenOStream("include", "any");
			if (pStream) {
				anyIncl.Export(*pStream);
				delete pStream;
			}
		}
		{
			IStringStream is(strMain);
			anyMain.Import(is);
		}
		{
			IStringStream is(strRef);
			anyRef.Import(is);
		}
		assertAnyEqual(anyRef, anyMain);
	}
	{
		// Test an include with a query for a subtree in the include file that does not exist
		Anything anyMain, anyIncl, anyRef;

		String strMain(_QUOTE_( { /200 { /a gugus /b gaga } !"file:///include.any?d:0" }));
		String strIncl(_QUOTE_( { /100 { /d { foo } /e frim } }));
		String strRef (_QUOTE_( { /200 { /a gugus /b gaga } * }));
		{
			IStringStream is(strIncl);
			anyIncl.Import(is);
			iostream *pStream = System::OpenOStream("include", "any");
			if (pStream) {
				anyIncl.Export(*pStream);
				delete pStream;
			}
		}
		{
			IStringStream is(strMain);
			anyMain.Import(is);
		}
		{
			IStringStream is(strRef);
			anyRef.Import(is);
		}
		assertAnyEqual(anyRef, anyMain);
	}
}

void AnythingTest::DeepClone0Test()
{
	Anything anyOriginal, anyClone;
	Anything AnyLong( 5L );
	Anything AnyVoidBuff( (void *)"Bytebuffer", 10 );
	String   MyString( "Another String" );
	int      MyInt = 34;
	long     MyLong = 56;
	long	 typeBefore, sizeBefore;

	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	// ...
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );

	anyOriginal = (char *)"Ein String";
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( strcmp( anyOriginal.AsCharPtr(), "Ein String" ) == 0 );
	t_assert( strcmp( anyClone.AsCharPtr(), "Ein String" ) == 0  );

	anyOriginal = (long)MyInt;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	assertCompare( anyOriginal.AsLong(), equal_to, (long)MyInt );
	assertCompare( anyClone.AsLong(), equal_to, (long)MyInt );

	anyOriginal.Append( 3L );
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( anyOriginal[0L].AsLong() == MyInt );
	t_assert( anyClone[0L].AsLong() == MyInt );
	t_assert( anyOriginal[1].AsLong() == 3 );
	t_assert( anyClone[1].AsLong() == 3 );

	//PS: try new assertAnyEqual
	assertAnyEqual(anyOriginal, anyClone);

	anyOriginal.Append( "Noch ein String" );
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( anyOriginal[0L].AsLong() == MyInt );
	t_assert( anyClone[0L].AsLong() == MyInt );
	t_assert( anyOriginal[1].AsLong() == 3 );
	t_assert( anyClone[1].AsLong() == 3 );
	t_assert( strcmp( anyOriginal[2].AsCharPtr(), "Noch ein String" ) == 0 );
	t_assert( strcmp( anyClone[2].AsCharPtr(), "Noch ein String" ) == 0  );

	anyOriginal = MyLong;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	assertCompare( anyOriginal.AsLong(), equal_to, MyLong );
	assertCompare( anyClone.AsLong(), equal_to, MyLong );

	anyOriginal = (double)300.44;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	assertCompare( anyOriginal.AsDouble(), equal_to, 300.44 );
	assertCompare( anyClone.AsDouble(), equal_to, 300.44 );

	anyOriginal = MyString;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( strcmp( anyOriginal.AsCharPtr(), (const char *)MyString ) == 0 );
	t_assert( strcmp( anyClone.AsCharPtr(), (const char *)MyString ) == 0  );

	anyOriginal = 1.37;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	assertCompare( anyOriginal.AsDouble(), equal_to, 1.37 );
	assertCompare( anyClone.AsDouble(), equal_to, 1.37 );

	anyOriginal = AnyLong;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	assertCompare( anyOriginal.AsLong(), equal_to, AnyLong.AsLong() );
	assertCompare( anyClone.AsLong(), equal_to, AnyLong.AsLong() );

	anyOriginal = AnythingTest::init5DimArray(5);
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	t_assert( anyClone.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	assertCompare( anyClone.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( AnythingTest::check5DimArray( anyOriginal, anyClone, 5 ) == true );
}

void AnythingTest::DeepClone1Test()
{
	Anything anyOriginal;
	Anything AnyLong( 5L );
	Anything AnyVoidBuff( (void *)"Bytebuffer", 10 );
	String   MyString( "Another String" );
	int      MyInt = 34;
	long     MyLong = 56;
	long	 typeBefore, sizeBefore;

	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	// ...

	anyOriginal = (char *)"Ein String";
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( strcmp( anyOriginal.AsCharPtr(), "Ein String" ) == 0 );

	anyOriginal = (long)MyInt;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( anyOriginal.AsLong() == MyInt );

	anyOriginal.Append( 3L );
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( anyOriginal[0L].AsLong() == MyInt );
	t_assert( anyOriginal[1].AsLong() == 3 );

	anyOriginal.Append( "Noch ein String" );
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( anyOriginal[0L].AsLong() == MyInt );
	t_assert( anyOriginal[1].AsLong() == 3 );
	t_assert( strcmp( anyOriginal[2].AsCharPtr(), "Noch ein String" ) == 0 );

	anyOriginal = MyLong;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	assertCompare( anyOriginal.AsLong(), equal_to, MyLong );

	anyOriginal = (double)300.44;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	assertCompare( anyOriginal.AsDouble(), equal_to, 300.44 );

	anyOriginal = MyString;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( strcmp( anyOriginal.AsCharPtr(), (const char *)MyString ) == 0 );

	anyOriginal = 1.37;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	assertCompare( anyOriginal.AsDouble(), equal_to, 1.37 );

	anyOriginal = AnyLong;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	assertCompare( anyOriginal.AsLong(), equal_to, AnyLong.AsLong() );

	anyOriginal = AnythingTest::init5DimArray(5);
	Anything anySave;
	anySave = anyOriginal;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( AnythingTest::check5DimArray( anyOriginal, anySave, 5 ) == true );
}

void AnythingTest::DeepClone2Test()
{
	Anything	anyOriginal, any0;
	long	 	i, typeBefore, sizeBefore;

	for ( i = 0; i < 100; i++ ) {
		anyOriginal.Append(i);
	}
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();

	any0 = anyOriginal;
	any0[100] = any0.DeepClone();
	// Check type of anyOriginal
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check size anyOriginal
	t_assert( anyOriginal.GetSize() == sizeBefore + 1 );

	// Check type of any0
	t_assert( any0.GetType() == AnyArrayType );
	// Check size anyOriginal
	t_assert( any0.GetSize() == sizeBefore + 1 );
	// Check values of any0
	for ( i = 0; i < 100; i++ ) {
		t_assert( any0[i] == i );
	}
	for ( i = 0; i < 100; i++ ) {
		t_assert( any0[100][i] == i );
	}
}

void AnythingTest::DeepClone3Test()
{
	Anything	anyOriginal, any0;
	long	 	i, typeBefore, sizeBefore;

	for ( i = 0; i < 100; i++ ) {
		anyOriginal.Append(i);
	}
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();

	any0 = anyOriginal;
	any0[99] = any0.DeepClone();
	// Check type of anyOriginal
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check size anyOriginal
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );

	// Check type of any0
	t_assert( any0.GetType() == AnyArrayType );
	// Check size anyOriginal
	assertCompare( any0.GetSize(), equal_to, sizeBefore );
	// Check values of any0
	for ( i = 0; i < 99; i++ ) {
		t_assert( any0[i] == i );
	}
	for ( i = 0; i < 100; i++ ) {
		t_assert( any0[99][i] == i );
	}
}

void AnythingTest::String2LongConversion()
{
	Anything a("1212");
	long l = a.AsLong(-1);
	assertEqual(1212, l);
}

void AnythingTest::DeepClone4Test()
{
	Anything	any0, any1;
	long	 	typeBefore, sizeBefore;

	any0.Append(12L);
	any0.Append(13L);
	any0.Remove(1);

	typeBefore = any0.GetType();
	sizeBefore = any0.GetSize();

	any1 = any0[1].DeepClone();
	// the access increases the size by one

	// Check type
	t_assert( any0.GetType() == typeBefore );
	// Check size
	assertEqual( sizeBefore + 1, any0.GetSize() );
	// Check value
	assertEqual( 12, any0[0L].AsLong(-1) );
	assertEqual( -2, any0.AsLong(-2) );

	// check clone
	t_assert( any1.GetType() == AnyNullType );
}

void AnythingTest::DeepClone5Test()
{
	Anything	any0, any1 = 1L;
	t_assert( any0.GetType() == AnyNullType );
	t_assert( any1.GetType() == AnyLongType );

	any1 = any0.DeepClone();
	t_assert( any1.GetType() == AnyNullType );
} // testeDeepClone5

void AnythingTest::ReadFailsTest()
{
	String incompleteAny("{ /Slot { /No \"Ending curly bracket\"");
	IStringStream is(incompleteAny);
	Anything any;
	t_assert(!any.Import(is));
	assertEqual("Ending curly bracket", any["Slot"]["No"].AsString("x"));
}

void AnythingTest::WriteReadTest()
{
	StartTraceMem(AnythingTest.WriteReadTest);
	WriteRead0Test();
	WriteRead1Test();
	WriteRead5Test();
	WriteRead7Test();
	WriteRead8Test();
	DeepClone0Test();
	DeepClone1Test();
	DeepClone2Test();
	DeepClone3Test();
	DeepClone4Test();
	DeepClone5Test();
}

/*================================================================================================================*/
/*                       I N P U T / O U T P U T    T E S T S   end                                               */
/*================================================================================================================*/

/*================================================================================================================*/
/*                           L O O K U P P A T H    T E S T S   begin                                             */
/*================================================================================================================*/
void AnythingTest::LookUp0Test()
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

void AnythingTest::LookUp1Test()
{
	long		i0, i1;
	char		idx0[3] = {0}, idx1[3] = {0};
	String		path;
	Anything	any0, any1, anyTest, derived;

	// Init 'any0'
	any0 = AnythingTest::init5DimArray(5);

	// Reinit 'any0' using LookupPath
	for ( i0 = '0'; i0 < '5'; i0++ ) {
		long l0 = (long)(i0 - '0');
		idx0[0L] = (char)i0;
		for ( i1 = '0'; i1 < '5'; i1++ ) {
			long l1 = (long)(i1 - '0');
			idx1[0L] = (char)i1;
			path = idx0;
			t_assert(any0.LookupPath( derived, path ));
			derived[idx1] = 2 * ((l0) + (l1)) + 1;
		}
	}

	// Test if reinit was OK
	for ( i0 = '0'; i0 < '5'; i0++ ) {
		long l0 = (long)(i0 - '0');
		idx0[0L] = (char)i0;
		for ( i1 = '0'; i1 < '5'; i1++ ) {
			long l1 = (long)(i1 - '0');
			idx1[0L] = (char)i1;
			assertEqual(2 * ((l0) + (l1)) + 1, any0[idx0][idx1].AsLong());
		}
	}
}

void AnythingTest::LookupPathByIndex()
{
	{
		// Index at the end
		Anything 	test;
		Anything 	result;
		ROAnything	roTest;
		ROAnything	roResult;

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
		ROAnything	roTest;
		ROAnything	roResult;

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

void AnythingTest::EmptyLookup ()
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

void AnythingTest::invPathLookup ()
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

void AnythingTest::intLookupPathCheck(Anything &test, const char *path)
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

void AnythingTest::LookUpWithSpecialCharsTest()
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

void AnythingTest::LookupCaseSensitiveTest()
{
	Anything any;
	Anything res;
	any["ALL"] = "content";
	t_assert(any.LookupPath(res, "all") == 0);
}

void AnythingTest::LookUpTest()
{
	LookUp0Test();
	LookUp1Test();
	LookupPathByIndex();
	EmptyLookup();
	invPathLookup();
	LookUpWithSpecialCharsTest();
	LookupCaseSensitiveTest();
}

/*================================================================================================================*/
/*                           L O O K U P P A T H    T E S T S   end                                               */
/*================================================================================================================*/

void AnythingTest::SuccessiveAssignments()
// some random assignments with a strange outcome
{
	Anything a = "Test";
	t_assert(!a.IsDefined("Test"));
	assertEqual("Test", a.AsCharPtr("nothing"));

	if ( a.IsDefined("Test") ) {
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
	b.Append(sub);		// -> AnyArrayImpl
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
	assertEqual(i,  intAny.AsLong(0));

	Anything b = intAny;

	i = -10;
	intAny = i;

	assertEqual(i,  intAny.AsLong(0));
	assertEqual(b.AsLong(0),  10L);	// still 10
}

void AnythingTest::ifaObjectOperatorAssign()
{
	DummyIFAObj o("test");
	Anything objAny = &o;

	assertEqual((long)(&o), (long) objAny.AsIFAObject(0));
	assertEqual((long)(&o), (long) objAny.AsLong(0));
	assertEqual((long)0.0, (long) objAny.AsDouble(0.0));
	assertEqual("IFAObject",  objAny.AsCharPtr(""));
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
	assertEqual(i,  roIntAny.AsLong(0));

	i = -10;
	intAny = i;
	roIntAny = intAny;

	assertEqual(i,  roIntAny.AsLong(0));
}

void AnythingTest::ifaObjectROConversion()
{
	DummyIFAObj o("test");
	Anything objAny = &o;
	ROAnything roObjAny(objAny);

	assertEqual((long)(&o), (long) roObjAny.AsIFAObject(0));
	assertEqual((long)(&o), (long) roObjAny.AsLong(0));
	assertEqual((long)0.0, (long) roObjAny.AsDouble(0.0));
	assertEqual("IFAObject",  roObjAny.AsCharPtr(""));
	assertEqual((long)(false), (long) roObjAny.AsBool(false));
}

void AnythingTest::BinaryBufOutput()
{
	Anything testAny;
	testAny["T1"] = Anything( (void *)"1234567890", 10 );
	String testString;
	OStringStream os(&testString);

	os << testAny << flush;

	assertEqual( "{\n  /T1 [10;1234567890]\n}", testString);
	assertEqual(25, testString.Length());
}

void AnythingTest::AsCharPtrBufLen()
{
	// test AsCharPtr(const char *dft, long &buflen)

	//--- AnyArrayImpl
	MetaThing anyArray;
	long arraylen;
	anyArray.AsCharPtr("", arraylen);
	assertEqual(12, arraylen);	// strlen("AnyArrayImpl")

	//--- AnyBinaryBufImpl
	char test[10];
	memset(test, '\0', 10);
	Anything anyTest( (void *)test, 10);
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

void AnythingTest::DeepCloneWithRef()
{
	Anything a;
	Anything b;
	Anything c;
	Anything d = "Sommerlad";
	b[0L] = "hallo";
	b[1L] = "Peter";
	b[2L] = d;
	b[3L] = d;

	a["slot1"] = b;
	a["slot2"] = b;
	assertEqual("hallo", a["slot1"][0L].AsCharPtr("no"));
	assertEqual("hallo", a["slot2"][0L].AsCharPtr("no"));
	assertEqual("Peter", a["slot1"][1L].AsCharPtr("no"));
	assertEqual("Peter", a["slot2"][1L].AsCharPtr("no"));
	assertEqual("Sommerlad", a["slot1"][2L].AsCharPtr("no"));
	assertEqual("Sommerlad", a["slot2"][3L].AsCharPtr("no"));
	assertEqual((long)(a["slot1"][2L].GetImpl()), (long)(a["slot2"][2L].GetImpl()));
	assertEqual((long)(a["slot1"][3L].GetImpl()), (long)(a["slot2"][3L].GetImpl()));
	d = "Birrer";
	assertEqual("Sommerlad", a["slot1"][2L].AsCharPtr("no"));
	assertEqual("Sommerlad", a["slot2"][3L].AsCharPtr("no"));
	a["slot1"][2L] = "Birrer";
	assertEqual("Birrer", a["slot1"][2L].AsCharPtr("no"));
	assertEqual("Sommerlad", a["slot2"][3L].AsCharPtr("no"));

	b[0L] = "tschau";

	assertEqual("tschau", a["slot1"][0L].AsCharPtr("no"));
	assertEqual("tschau", a["slot2"][0L].AsCharPtr("no"));

	t_assert(((long)(a["slot1"].GetImpl()) != 0));
	assertEqual((long)(a["slot1"].GetImpl()), (long)(a["slot2"].GetImpl()));
	assertEqual((long)(a["slot1"][0L].GetImpl()), (long)(a["slot2"][0L].GetImpl()));
	assertEqual((long)(a["slot1"][1L].GetImpl()), (long)(a["slot2"][1L].GetImpl()));
	c = a.DeepClone();
	assertEqual((long)(c["slot1"].GetImpl()), (long)(c["slot2"].GetImpl()));
	assertEqual((long)(c["slot1"][0L].GetImpl()), (long)(c["slot2"][0L].GetImpl()));
	assertEqual((long)(c["slot1"][1L].GetImpl()), (long)(c["slot2"][1L].GetImpl()));

	c["slot1"][0L] = "hallo";
	assertEqual("hallo", c["slot2"][0L].AsCharPtr("no"));
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

		SlotCopier::Operate(source, result, copyList, config["Delim"].AsCharPtr(".")[0L], config["IndexDelim"].AsCharPtr(":")[0L]);

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
	Anything expectedStore(fConfig["Results"][name()]);
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
	if ( t_assert(((ROAnything)fConfig).LookupPath(roaTestConfig, "SlotnameSorterTest")) ) {
		ROAnything cConfig(roaTestConfig[0L]);
		for (long lIdx = 0; lIdx < roaTestConfig.GetSize(); lIdx++, cConfig = roaTestConfig[lIdx]) {
			TraceAny(cConfig, "the config");
			Anything sorted;
			SlotnameSorter::EMode mode(cConfig["Mode"].AsString() == "asc" ? SlotnameSorter::asc : SlotnameSorter::desc);
			sorted = cConfig["TestArray"].DeepClone();
			SlotnameSorter::Sort(sorted, mode);
			assertAnyEqual(	cConfig["ExpectedResult"], sorted);
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
