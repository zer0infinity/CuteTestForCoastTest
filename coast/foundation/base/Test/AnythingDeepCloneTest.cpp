/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnythingDeepCloneTest.h"
#include "TestSuite.h"
#include "StringStream.h"
#include "IFAObject.h"
#include "Dbg.h"
#include "AnyIterators.h"
#include "SystemLog.h"
#include "PoolAllocator.h"
#include <cstring>

//---- AnythingDeepCloneTest ---------------------------------------------------------

AnythingDeepCloneTest::AnythingDeepCloneTest(TString tname) :
	TestCaseType(tname)
{
}

void AnythingDeepCloneTest::setUp()
{
	StartTrace(AnythingDeepCloneTest.setUp);
}

Test *AnythingDeepCloneTest::suite()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnythingDeepCloneTest, DeepClone0Test);
	ADD_CASE(testSuite, AnythingDeepCloneTest, DeepClone1Test);
	ADD_CASE(testSuite, AnythingDeepCloneTest, DeepClone2Test);
	ADD_CASE(testSuite, AnythingDeepCloneTest, DeepClone3Test);
	ADD_CASE(testSuite, AnythingDeepCloneTest, DeepClone4Test);
	ADD_CASE(testSuite, AnythingDeepCloneTest, DeepClone5Test);
	ADD_CASE(testSuite, AnythingDeepCloneTest, DeepCloneWithRef);
	ADD_CASE(testSuite, AnythingDeepCloneTest, DeepCloneBug232Test);
	return testSuite;
}

Anything AnythingDeepCloneTest::init5DimArray(long anzElt)
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

bool AnythingDeepCloneTest::check5DimArray(Anything &any0, Anything &any1, long anzElt)
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

void AnythingDeepCloneTest::DeepClone0Test()
{
	Anything anyOriginal, anyClone;
	Anything AnyLong(5L);
	Anything AnyVoidBuff((void *) "Bytebuffer", 10);
	String MyString("Another String");
	int MyInt = 34;
	long MyLong = 56;
	long typeBefore, sizeBefore;

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

	anyOriginal = (char *) "Ein String";
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
	t_assert( strcmp( anyClone.AsCharPtr(), "Ein String" ) == 0 );

	anyOriginal = (long) MyInt;
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

	anyOriginal.Append(3L);
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

	anyOriginal.Append("Noch ein String");
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
	t_assert( strcmp( anyClone[2].AsCharPtr(), "Noch ein String" ) == 0 );

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

	anyOriginal = (double) 300.44;
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
	t_assert( strcmp( anyClone.AsCharPtr(), (const char *)MyString ) == 0 );

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

	anyOriginal = AnythingDeepCloneTest::init5DimArray(5);
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
	t_assert( AnythingDeepCloneTest::check5DimArray( anyOriginal, anyClone, 5 ) == true );
}

void AnythingDeepCloneTest::DeepClone1Test()
{
	Anything anyOriginal;
	Anything AnyLong(5L);
	Anything AnyVoidBuff((void *) "Bytebuffer", 10);
	String MyString("Another String");
	int MyInt = 34;
	long MyLong = 56;
	long typeBefore, sizeBefore;

	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	// ...

	anyOriginal = (char *) "Ein String";
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( strcmp( anyOriginal.AsCharPtr(), "Ein String" ) == 0 );

	anyOriginal = (long) MyInt;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	t_assert( anyOriginal.GetType() == typeBefore );
	// Check sizes
	assertCompare( anyOriginal.GetSize(), equal_to, sizeBefore );
	// Check values
	t_assert( anyOriginal.AsLong() == MyInt );

	anyOriginal.Append(3L);
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

	anyOriginal.Append("Noch ein String");
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

	anyOriginal = (double) 300.44;
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

	anyOriginal = AnythingDeepCloneTest::init5DimArray(5);
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
	t_assert( AnythingDeepCloneTest::check5DimArray( anyOriginal, anySave, 5 ) == true );
}

void AnythingDeepCloneTest::DeepClone2Test()
{
	Anything anyOriginal, any0;
	long i, typeBefore, sizeBefore;

	for (i = 0; i < 100; i++) {
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
	for (i = 0; i < 100; i++) {
		t_assert( any0[i] == i );
	}
	for (i = 0; i < 100; i++) {
		t_assert( any0[100][i] == i );
	}
}

void AnythingDeepCloneTest::DeepClone3Test()
{
	Anything anyOriginal, any0;
	long i, typeBefore, sizeBefore;

	for (i = 0; i < 100; i++) {
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
	for (i = 0; i < 99; i++) {
		t_assert( any0[i] == i );
	}
	for (i = 0; i < 100; i++) {
		t_assert( any0[99][i] == i );
	}
}

void AnythingDeepCloneTest::DeepClone4Test()
{
	Anything any0, any1;
	long typeBefore, sizeBefore;

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

void AnythingDeepCloneTest::DeepClone5Test()
{
	Anything any0, any1 = 1L;
	t_assert( any0.GetType() == AnyNullType );
	t_assert( any1.GetType() == AnyLongType );

	any1 = any0.DeepClone();
	t_assert( any1.GetType() == AnyNullType );
} // testeDeepClone5

void AnythingDeepCloneTest::DeepCloneWithRef()
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
//	assertEqual((long)(a["slot1"][2L].GetImpl()), (long)(a["slot2"][2L].GetImpl()));
//	assertEqual((long)(a["slot1"][3L].GetImpl()), (long)(a["slot2"][3L].GetImpl()));
	d = "Birrer";
	assertEqual("Sommerlad", a["slot1"][2L].AsCharPtr("no"));
	assertEqual("Sommerlad", a["slot2"][3L].AsCharPtr("no"));
	a["slot1"][2L] = "Birrer";
	assertEqual("Birrer", a["slot1"][2L].AsCharPtr("no"));
	assertEqual("Sommerlad", a["slot2"][3L].AsCharPtr("no"));

	b[0L] = "tschau";

	assertEqual("tschau", a["slot1"][0L].AsCharPtr("no"));
	assertEqual("tschau", a["slot2"][0L].AsCharPtr("no"));

//	t_assert(((long)(a["slot1"].GetImpl()) != 0));
//	assertEqual((long)(a["slot1"].GetImpl()), (long)(a["slot2"].GetImpl()));
//	assertEqual((long)(a["slot1"][0L].GetImpl()), (long)(a["slot2"][0L].GetImpl()));
//	assertEqual((long)(a["slot1"][1L].GetImpl()), (long)(a["slot2"][1L].GetImpl()));
	c = a.DeepClone();
//	assertEqual((long)(c["slot1"].GetImpl()), (long)(c["slot2"].GetImpl()));
//	assertEqual((long)(c["slot1"][0L].GetImpl()), (long)(c["slot2"][0L].GetImpl()));
//	assertEqual((long)(c["slot1"][1L].GetImpl()), (long)(c["slot2"][1L].GetImpl()));

	c["slot1"][0L] = "hallo";
	assertEqual("hallo", c["slot2"][0L].AsCharPtr("no"));
}

void AnythingDeepCloneTest::DeepCloneBug232Test() {
	StartTrace(AnythingDeepCloneTest.DeepCloneBug232Test);
	{
		// test escaped reference
		String str(_QUOTE_( { /level { /BackendShortName "avt" /BackendName { %level.BackendShortName /Lookup RenderSSODomain } /SSOBackendName { /Lookup RenderSSOPrefix "." %level.BackendName } /ServiceLink { /URL { { /Lookup RenderSSOProtocol } "://" %level.SSOBackendName { /Lookup RenderSSOPort } "/" } /Name "AVT" /Tooltip "Betrieb" /DisplayLoc "ExtServiceLinkList" /DisplayOrder "20" } } }));
		Anything anyExpected(Coast::Storage::Global());
		IStringStream is(str);
		anyExpected.Import(is);
		TraceAny(anyExpected, "imported and expected");
		ROAnything roaEntry = anyExpected;
		assertAnyEqualm(anyExpected, roaEntry, "expected ROAnything wrapper to match underlying");
		TraceAny(roaEntry, "ROAnything");
		PoolAllocator p(1, 16384, 20);
		TestStorageHooks tsh(&p);
		Anything anyResult(&p);
		anyResult = roaEntry.DeepClone(&p);
		TraceAny(anyResult, "cloned");
		assertAnyEqualm(anyExpected, anyResult, "expected cloned any of ROAnything to match");
	}
	{
		// test escaped reference
		String str(_QUOTE_( { /level { /BackendShortName "avt" /BackendName { %level.BackendShortName /Lookup RenderSSODomain } /SSOBackendName { /Lookup RenderSSOPrefix "." %level.BackendName } /ServiceLink { /URL { { /Lookup RenderSSOProtocol } "://" %level.SSOBackendName { /Lookup RenderSSOPort } "/" } /Name "AVT" /Tooltip "Betrieb" /DisplayLoc "ExtServiceLinkList" /DisplayOrder "20" } } }));
		Anything anyExpected(Coast::Storage::Global());
		IStringStream is(str);
		anyExpected.Import(is);
		TraceAny(anyExpected, "imported and expected");
		ROAnything roaEntry = anyExpected;
		assertAnyEqualm(anyExpected, roaEntry, "expected ROAnything wrapper to match underlying");
		TraceAny(roaEntry, "ROAnything");
		PoolAllocator p(1, 16384, 20);
		TestStorageHooks tsh(&p);
		Anything anyResult(&p);
		anyResult = roaEntry["level"]["ServiceLink"].DeepClone(&p);
		TraceAny(anyResult, "cloned");
		assertAnyEqualm(anyExpected["level"]["ServiceLink"], anyResult, "expected cloned any of ROAnything to match");
	}
}
