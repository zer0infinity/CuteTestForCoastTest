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

#include "AnythingDeepCloneTest.h"
#include "PoolAllocator.h"

void AnythingDeepCloneTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(AnythingDeepCloneTest, DeepClone0Test));
	s.push_back(CUTE_SMEMFUN(AnythingDeepCloneTest, DeepClone1Test));
	s.push_back(CUTE_SMEMFUN(AnythingDeepCloneTest, DeepClone2Test));
	s.push_back(CUTE_SMEMFUN(AnythingDeepCloneTest, DeepClone3Test));
	s.push_back(CUTE_SMEMFUN(AnythingDeepCloneTest, DeepClone4Test));
	s.push_back(CUTE_SMEMFUN(AnythingDeepCloneTest, DeepClone5Test));
	s.push_back(CUTE_SMEMFUN(AnythingDeepCloneTest, DeepCloneWithRef));
	s.push_back(CUTE_SMEMFUN(AnythingDeepCloneTest, DeepCloneBug232Test));
}

Anything AnythingDeepCloneTest::init5DimArray(long anzElt) {
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

bool AnythingDeepCloneTest::check5DimArray(Anything &any0, Anything &any1, long anzElt) {
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

void AnythingDeepCloneTest::DeepClone0Test() {
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
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	// ...
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );

	anyOriginal = (char *) "Ein String";
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	ASSERT( strcmp( anyOriginal.AsCharPtr(), "Ein String" ) == 0 );
	ASSERT( strcmp( anyClone.AsCharPtr(), "Ein String" ) == 0 );

	anyOriginal = (long) MyInt;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	ASSERT_EQUAL( anyOriginal.AsLong(), (long)MyInt );
	ASSERT_EQUAL( anyClone.AsLong(), (long)MyInt );

	anyOriginal.Append(3L);
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	ASSERT( anyOriginal[0L].AsLong() == MyInt );
	ASSERT( anyClone[0L].AsLong() == MyInt );
	ASSERT( anyOriginal[1].AsLong() == 3 );
	ASSERT( anyClone[1].AsLong() == 3 );

	//PS: try new ASSERT_EQUAL
	ASSERT_EQUAL(anyOriginal, anyClone);

	anyOriginal.Append("Noch ein String");
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	ASSERT( anyOriginal[0L].AsLong() == MyInt );
	ASSERT( anyClone[0L].AsLong() == MyInt );
	ASSERT( anyOriginal[1].AsLong() == 3 );
	ASSERT( anyClone[1].AsLong() == 3 );
	ASSERT( strcmp( anyOriginal[2].AsCharPtr(), "Noch ein String" ) == 0 );
	ASSERT( strcmp( anyClone[2].AsCharPtr(), "Noch ein String" ) == 0 );

	anyOriginal = MyLong;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	ASSERT_EQUAL( anyOriginal.AsLong(), MyLong );
	ASSERT_EQUAL( anyClone.AsLong(), MyLong );

	anyOriginal = (double) 300.44;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	ASSERT_EQUAL( anyOriginal.AsDouble(), 300.44 );
	ASSERT_EQUAL( anyClone.AsDouble(), 300.44 );

	anyOriginal = MyString;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	ASSERT( strcmp( anyOriginal.AsCharPtr(), (const char *)MyString ) == 0 );
	ASSERT( strcmp( anyClone.AsCharPtr(), (const char *)MyString ) == 0 );

	anyOriginal = 1.37;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	ASSERT_EQUAL( anyOriginal.AsDouble(), 1.37 );
	ASSERT_EQUAL( anyClone.AsDouble(), 1.37 );

	anyOriginal = AnyLong;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	ASSERT_EQUAL( anyOriginal.AsLong(), AnyLong.AsLong() );
	ASSERT_EQUAL( anyClone.AsLong(), AnyLong.AsLong() );

	anyOriginal = AnythingDeepCloneTest::init5DimArray(5);
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyClone = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	ASSERT( anyClone.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	ASSERT_EQUAL( anyClone.GetSize(), sizeBefore );
	// Check values
	ASSERT( AnythingDeepCloneTest::check5DimArray( anyOriginal, anyClone, 5 ) == true );
}

void AnythingDeepCloneTest::DeepClone1Test() {
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
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	// ...

	anyOriginal = (char *) "Ein String";
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	ASSERT( strcmp( anyOriginal.AsCharPtr(), "Ein String" ) == 0 );

	anyOriginal = (long) MyInt;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	ASSERT( anyOriginal.AsLong() == MyInt );

	anyOriginal.Append(3L);
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	ASSERT( anyOriginal[0L].AsLong() == MyInt );
	ASSERT( anyOriginal[1].AsLong() == 3 );

	anyOriginal.Append("Noch ein String");
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	ASSERT( anyOriginal[0L].AsLong() == MyInt );
	ASSERT( anyOriginal[1].AsLong() == 3 );
	ASSERT( strcmp( anyOriginal[2].AsCharPtr(), "Noch ein String" ) == 0 );

	anyOriginal = MyLong;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	ASSERT_EQUAL( anyOriginal.AsLong(), MyLong );

	anyOriginal = (double) 300.44;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	ASSERT_EQUAL( anyOriginal.AsDouble(), 300.44 );

	anyOriginal = MyString;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	ASSERT( strcmp( anyOriginal.AsCharPtr(), (const char *)MyString ) == 0 );

	anyOriginal = 1.37;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	ASSERT_EQUAL( anyOriginal.AsDouble(), 1.37 );

	anyOriginal = AnyLong;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	ASSERT_EQUAL( anyOriginal.AsLong(), AnyLong.AsLong() );

	anyOriginal = AnythingDeepCloneTest::init5DimArray(5);
	Anything anySave;
	anySave = anyOriginal;
	typeBefore = anyOriginal.GetType();
	sizeBefore = anyOriginal.GetSize();
	anyOriginal = anyOriginal.DeepClone();
	// Check types
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check sizes
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );
	// Check values
	ASSERT( AnythingDeepCloneTest::check5DimArray( anyOriginal, anySave, 5 ) == true );
}

void AnythingDeepCloneTest::DeepClone2Test() {
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
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check size anyOriginal
	ASSERT( anyOriginal.GetSize() == sizeBefore + 1 );

	// Check type of any0
	ASSERT( any0.GetType() == AnyArrayType );
	// Check size anyOriginal
	ASSERT( any0.GetSize() == sizeBefore + 1 );
	// Check values of any0
	for (i = 0; i < 100; i++) {
		ASSERT( any0[i] == i );
	}
	for (i = 0; i < 100; i++) {
		ASSERT( any0[100][i] == i );
	}
}

void AnythingDeepCloneTest::DeepClone3Test() {
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
	ASSERT( anyOriginal.GetType() == typeBefore );
	// Check size anyOriginal
	ASSERT_EQUAL( anyOriginal.GetSize(), sizeBefore );

	// Check type of any0
	ASSERT( any0.GetType() == AnyArrayType );
	// Check size anyOriginal
	ASSERT_EQUAL( any0.GetSize(), sizeBefore );
	// Check values of any0
	for (i = 0; i < 99; i++) {
		ASSERT( any0[i] == i );
	}
	for (i = 0; i < 100; i++) {
		ASSERT( any0[99][i] == i );
	}
}

void AnythingDeepCloneTest::DeepClone4Test() {
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
	ASSERT( any0.GetType() == typeBefore );
	// Check size
	ASSERT_EQUAL( sizeBefore + 1, any0.GetSize() );
	// Check value
	ASSERT_EQUAL( 12, any0[0L].AsLong(-1) );
	ASSERT_EQUAL( -2, any0.AsLong(-2) );

	// check clone
	ASSERT( any1.GetType() == AnyNullType );
}

void AnythingDeepCloneTest::DeepClone5Test() {
	Anything any0, any1 = 1L;
	ASSERT( any0.GetType() == AnyNullType );
	ASSERT( any1.GetType() == AnyLongType );

	any1 = any0.DeepClone();
	ASSERT( any1.GetType() == AnyNullType );
} // testeDeepClone5

void AnythingDeepCloneTest::DeepCloneWithRef() {
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
	ASSERT_EQUAL("hallo", a["slot1"][0L].AsCharPtr("no"));
	ASSERT_EQUAL("hallo", a["slot2"][0L].AsCharPtr("no"));
	ASSERT_EQUAL("Peter", a["slot1"][1L].AsCharPtr("no"));
	ASSERT_EQUAL("Peter", a["slot2"][1L].AsCharPtr("no"));
	ASSERT_EQUAL("Sommerlad", a["slot1"][2L].AsCharPtr("no"));
	ASSERT_EQUAL("Sommerlad", a["slot2"][3L].AsCharPtr("no"));
	//	ASSERT_EQUAL((long)(a["slot1"][2L].GetImpl()), (long)(a["slot2"][2L].GetImpl()));
	//	ASSERT_EQUAL((long)(a["slot1"][3L].GetImpl()), (long)(a["slot2"][3L].GetImpl()));
	d = "Birrer";
	ASSERT_EQUAL("Sommerlad", a["slot1"][2L].AsCharPtr("no"));
	ASSERT_EQUAL("Sommerlad", a["slot2"][3L].AsCharPtr("no"));
	a["slot1"][2L] = "Birrer";
	ASSERT_EQUAL("Birrer", a["slot1"][2L].AsCharPtr("no"));
	ASSERT_EQUAL("Sommerlad", a["slot2"][3L].AsCharPtr("no"));

	b[0L] = "tschau";

	ASSERT_EQUAL("tschau", a["slot1"][0L].AsCharPtr("no"));
	ASSERT_EQUAL("tschau", a["slot2"][0L].AsCharPtr("no"));

	//	ASSERT(((long)(a["slot1"].GetImpl()) != 0));
	//	ASSERT_EQUAL((long)(a["slot1"].GetImpl()), (long)(a["slot2"].GetImpl()));
	//	ASSERT_EQUAL((long)(a["slot1"][0L].GetImpl()), (long)(a["slot2"][0L].GetImpl()));
	//	ASSERT_EQUAL((long)(a["slot1"][1L].GetImpl()), (long)(a["slot2"][1L].GetImpl()));
	c = a.DeepClone();
	//	ASSERT_EQUAL((long)(c["slot1"].GetImpl()), (long)(c["slot2"].GetImpl()));
	//	ASSERT_EQUAL((long)(c["slot1"][0L].GetImpl()), (long)(c["slot2"][0L].GetImpl()));
	//	ASSERT_EQUAL((long)(c["slot1"][1L].GetImpl()), (long)(c["slot2"][1L].GetImpl()));

	c["slot1"][0L] = "hallo";
	ASSERT_EQUAL("hallo", c["slot2"][0L].AsCharPtr("no"));
}

void AnythingDeepCloneTest::DeepCloneBug232Test() {
	StartTrace(AnythingDeepCloneTest.DeepCloneBug232Test);
	{
		// test escaped reference
		String
				str(
						_QUOTE_( {	/level {/BackendShortName "avt" /BackendName {%level.BackendShortName /Lookup RenderSSODomain}/SSOBackendName {/Lookup RenderSSOPrefix "." %level.BackendName}/ServiceLink {/URL { {/Lookup RenderSSOProtocol}"://" %level.SSOBackendName {/Lookup RenderSSOPort}"/"}/Name "AVT" /Tooltip "Betrieb" /DisplayLoc "ExtServiceLinkList" /DisplayOrder "20"}}}));
		Anything anyExpected(coast::storage::Global());
		IStringStream is(str);
		anyExpected.Import(is);
		TraceAny(anyExpected, "imported and expected");
		ROAnything roaEntry = anyExpected;
		ASSERT_EQUALM( "expected ROAnything wrapper to match underlying",anyExpected, roaEntry);
		TraceAny(roaEntry, "ROAnything");
		PoolAllocator p(1, 16384, 20);
		TestStorageHooks tsh(&p);
		Anything anyResult(&p);
		anyResult = roaEntry.DeepClone(&p);
		TraceAny(anyResult, "cloned");
		ASSERT_EQUALM( "expected cloned any of ROAnything to match",anyExpected, anyResult);
	}
	{
		// test escaped reference
		String
				str(
						_QUOTE_( {	/level {/BackendShortName "avt" /BackendName {%level.BackendShortName /Lookup RenderSSODomain}/SSOBackendName {/Lookup RenderSSOPrefix "." %level.BackendName}/ServiceLink {/URL { {/Lookup RenderSSOProtocol}"://" %level.SSOBackendName {/Lookup RenderSSOPort}"/"}/Name "AVT" /Tooltip "Betrieb" /DisplayLoc "ExtServiceLinkList" /DisplayOrder "20"}}}));
		Anything anyExpected(coast::storage::Global());
		IStringStream is(str);
		anyExpected.Import(is);
		TraceAny(anyExpected, "imported and expected");
		ROAnything roaEntry = anyExpected;
		ASSERT_EQUALM( "expected ROAnything wrapper to match underlying",anyExpected, roaEntry);
		TraceAny(roaEntry, "ROAnything");
		PoolAllocator p(1, 16384, 20);
		TestStorageHooks tsh(&p);
		Anything anyResult(&p);
		anyResult = roaEntry["level"]["ServiceLink"].DeepClone(&p);
		TraceAny(anyResult, "cloned");
		ASSERT_EQUALM( "expected cloned any of ROAnything to match",anyExpected["level"]["ServiceLink"], anyResult);
	}
}
