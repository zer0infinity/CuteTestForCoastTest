/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "PoolAllocatorTest.h"

//--- module under test --------------------------------------------------------
#include "PoolAllocator.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- PoolAllocatorTest ----------------------------------------------------------------
PoolAllocatorTest::PoolAllocatorTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(PoolAllocatorTest.Ctor);
}

PoolAllocatorTest::~PoolAllocatorTest()
{
	StartTrace(PoolAllocatorTest.Dtor);
}

// uncomment if something special needs to be done which isnt already done in base class
//void PoolAllocatorTest::setUp()
//{
//	StartTrace(PoolAllocatorTest.setUp);
//}
//
//void PoolAllocatorTest::tearDown()
//{
//	StartTrace(PoolAllocatorTest.tearDown);
//}

//struct ExcessTrackerElt
//{
//	MemTracker *fpTracker;
//	ExcessTrackerElt *fpNext;
//	u_long fulBucketSize;
//	ExcessTrackerElt();
//
//	ExcessTrackerElt(MemTracker *pTracker, ExcessTrackerElt *pNext, u_long ulBucketSize);
//
//	~ExcessTrackerElt();
//
//	void PrintStatistic();
//
//	ul_long GetSizeToPowerOf2(u_long ulWishSize);
//
//	MemTracker* FindTrackerForSize(u_long lMemSize);
//
//	ExcessTrackerElt* InsertTrackerForSize(MemTracker *pTracker, u_long lMemSize);
//
//	void SetId(long lId);
//
//	MemTracker* operator[](u_long lMemSize);
//
//	l_long CurrentlyAllocated();
//
//	void Refresh();
//};

void PoolAllocatorTest::ExcessTrackerEltCtorTest()
{
	StartTrace(PoolAllocatorTest.ExcessTrackerEltCtorTest);
	ExcessTrackerElt aDfltCtor;
	t_assertm(aDfltCtor.fpTracker == NULL, "expected pointer to tracker to be NULL");
	t_assertm(aDfltCtor.fpNext == NULL, "expected pointer to next element to be NULL");
	assertEqualm(0, aDfltCtor.fulBucketSize, "expected default bucket size to be 0");
	ExcessTrackerElt aParamCtor((MemTracker *)0x12345, (ExcessTrackerElt *)0xabcde, 33);
	t_assertm(aParamCtor.fpTracker == (MemTracker *)0x12345, "expected pointer to be equal");
	t_assertm(aParamCtor.fpNext == (ExcessTrackerElt *)0xabcde, "expected pointer to next element to be equal");
	assertEqualm(33, aParamCtor.fulBucketSize, "expected default bucket size to be the same");
	ExcessTrackerElt aCopyCtor(aParamCtor);
	t_assertm(aCopyCtor.fpTracker == (MemTracker *)0x12345, "expected pointer to be equal");
	t_assertm(aCopyCtor.fpNext == (ExcessTrackerElt *)0xabcde, "expected pointer to next element to be equal");
	assertEqualm(33, aCopyCtor.fulBucketSize, "expected default bucket size to be the same");
	// need to reset tracker pointer and next pointer
	aParamCtor.fpTracker = NULL;
	aParamCtor.fpNext = NULL;
	aCopyCtor.fpTracker = NULL;
	aCopyCtor.fpNext = NULL;
}

void PoolAllocatorTest::ExcessTrackerEltGetSizeToPowerOf2Test()
{
	StartTrace(PoolAllocatorTest.ExcessTrackerEltGetSizeToPowerOf2Test);
	ExcessTrackerElt aDfltCtor;
	assertEqualm(4, aDfltCtor.GetSizeToPowerOf2(3), "expected correct size");
	assertEqualm(8, aDfltCtor.GetSizeToPowerOf2(8), "expected correct size");
	assertEqualm(16, aDfltCtor.GetSizeToPowerOf2(14), "expected correct size");
	assertEqualm(32, aDfltCtor.GetSizeToPowerOf2(17), "expected correct size");
	assertEqualm(32, aDfltCtor.GetSizeToPowerOf2(32), "expected correct size");
	assertEqualm(1048576, aDfltCtor.GetSizeToPowerOf2(524289), "expected correct size");
	assertEqualm(2147483648UL, aDfltCtor.GetSizeToPowerOf2(1073741825UL), "expected correct size");
}

void PoolAllocatorTest::ExcessTrackerEltFindTrackerForSizeTest()
{
	StartTrace(PoolAllocatorTest.ExcessTrackerEltFindTrackerForSizeTest);
	ExcessTrackerElt *pElt128 = new ExcessTrackerElt((MemTracker *)128, NULL, 128);
	ExcessTrackerElt *pElt64 = new ExcessTrackerElt((MemTracker *)64, pElt128, 64);
	ExcessTrackerElt *pElt32 = new ExcessTrackerElt((MemTracker *)32, pElt64, 32);
	t_assertm(pElt128->fpNext == 0, "expected pointer to be equal");
	assertEqualm(32, (long)pElt32->FindTrackerForSize(17), "expected correct tracker to be found");
	assertEqualm(32, (long)pElt32->FindTrackerForSize(32), "expected correct tracker to be found");
	assertEqualm(64, (long)pElt32->FindTrackerForSize(33), "expected correct tracker to be found");
	assertEqualm(64, (long)pElt32->FindTrackerForSize(64), "expected correct tracker to be found");
	assertEqualm(128, (long)pElt32->FindTrackerForSize(128), "expected correct tracker to be found");
	assertEqualm(0, (long)pElt32->FindTrackerForSize(230), "expected correct tracker to be found");
	pElt32->fpTracker = NULL;
	pElt64->fpTracker = NULL;
	pElt128->fpTracker = NULL;
	delete pElt32;
}

void PoolAllocatorTest::ExcessTrackerEltInsertTrackerForSizeTest()
{
	StartTrace(PoolAllocatorTest.ExcessTrackerEltInsertTrackerForSizeTest);

	ExcessTrackerElt aRoot;
	t_assert(&aRoot == aRoot.InsertTrackerForSize((MemTracker *)32, 32));
	t_assertm(aRoot.fpTracker == (MemTracker *)32, "expected pointer to be equal");
	t_assertm(aRoot.fpNext == NULL, "expected pointer to next element to be equal");
	assertEqualm(32, aRoot.fulBucketSize, "expected default bucket size to be the same");
	ExcessTrackerElt *pElt32 = &aRoot, *pElt64 = NULL, *pElt48 = NULL, *pElt16 = NULL;
	if ( t_assertm( ( pElt64 = aRoot.InsertTrackerForSize((MemTracker *)64, 64)) != NULL, "expected valid new element") ) {
		t_assertm( pElt64 != &aRoot, "expected new element not to be the root element");
		t_assertm(pElt64->fpTracker == (MemTracker *)64, "expected pointer to be equal");
		t_assertm(pElt64->fpNext == NULL, "expected pointer to next element to be equal");
		assertEqualm(64, pElt64->fulBucketSize, "expected default bucket size to be the same");
		t_assertm(aRoot.fpTracker == (MemTracker *)32, "expected pointer to be equal");
		t_assertm(aRoot.fpNext == pElt64, "expected pointer to next element to be equal");
		assertEqualm(32, aRoot.fulBucketSize, "expected default bucket size to be the same");
		if ( t_assertm( ( pElt48 = aRoot.InsertTrackerForSize((MemTracker *)48, 48)) != NULL, "expected valid new element") ) {
			t_assertm( pElt48 != &aRoot, "expected new element not to be the root element");
			t_assertm( pElt48 != pElt64, "expected new element not to be the root element");
			t_assertm(pElt48->fpTracker == (MemTracker *)48, "expected pointer to be equal");
			t_assertm(pElt48->fpNext == pElt64, "expected pointer to next element to be valid");
			assertEqualm(48, pElt48->fulBucketSize, "expected default bucket size to be the same");
			t_assertm(aRoot.fpTracker == (MemTracker *)32, "expected pointer to be equal");
			t_assertm(aRoot.fpNext == pElt48, "expected pointer to next element to be equal");
			assertEqualm(32, aRoot.fulBucketSize, "expected default bucket size to be the same");
			if ( t_assertm( ( pElt16 = aRoot.InsertTrackerForSize((MemTracker *)16, 16)) != NULL, "expected valid new element") ) {
				t_assertm( pElt16 == &aRoot, "expected new element not to be the root element");
				t_assertm(pElt16->fpTracker == (MemTracker *)16, "expected pointer to be equal");
				assertEqualm(16, pElt16->fulBucketSize, "expected default bucket size to be the same");
				t_assertm(aRoot.fpTracker == (MemTracker *)16, "expected pointer to be equal");
				assertEqualm(16, aRoot.fulBucketSize, "expected default bucket size to be the same");
				pElt32 = pElt16->fpNext;
				t_assertm(pElt32->fpTracker == (MemTracker *)32, "expected pointer to be equal");
				assertEqualm(32, pElt32->fulBucketSize, "expected default bucket size to be the same");
				t_assertm(pElt32->fpNext == pElt48, "expected pointer to next element to be valid");
				pElt16->fpTracker = NULL;
				pElt32->fpTracker = NULL;
			}
			pElt48->fpTracker = NULL;
		}
		pElt64->fpTracker = NULL;
	}
	aRoot.fpTracker = NULL;
}

void PoolAllocatorTest::ExcessTrackerEltTest()
{
	StartTrace(PoolAllocatorTest.ExcessTrackerEltTest);
	ExcessTrackerElt aRoot, *pElt32 = NULL, *pElt16 = NULL;
	MemTracker *pTrack16 = aRoot[16];
	Trace("trackername16 [" << pTrack16->fpName << "]");
	t_assertm(aRoot.fpTracker == pTrack16, "expected pointer to be equal");
	t_assertm(aRoot.fpNext == NULL, "expected pointer to next element to be equal");
	assertEqualm(16, aRoot.fulBucketSize, "expected default bucket size to be the same");
	MemTracker *pTrack32 = aRoot[32];
	Trace("trackername32 [" << pTrack32->fpName << "]");
	t_assertm(aRoot.fpTracker == pTrack16, "expected pointer to be equal");
	assertEqualm(16, aRoot.fulBucketSize, "expected default bucket size to be the same");
	pElt32 = aRoot.fpNext;
	t_assertm(pElt32->fpTracker == pTrack32, "expected pointer to be equal");
	assertEqualm(32, pElt32->fulBucketSize, "expected default bucket size to be the same");
	MemTracker *pTrack8 = aRoot[8];
	Trace("trackername8 [" << pTrack8->fpName << "]");
	t_assertm(aRoot.fpTracker == pTrack8, "expected pointer to be equal");
	assertEqualm(8, aRoot.fulBucketSize, "expected default bucket size to be the same");
	pElt16 = aRoot.fpNext;
	t_assertm(pElt16->fpTracker == pTrack16, "expected pointer to be equal");
	assertEqualm(16, pElt16->fulBucketSize, "expected default bucket size to be the same");
	t_assertm(pElt16->fpNext == pElt32, "expected pointer to next element to be equal");
	t_assertm(aRoot.fpTracker == pTrack8, "expected pointer to be equal");
	t_assertm(aRoot.fpNext == pElt16, "expected pointer to next element to be equal");
	assertEqualm(8, aRoot.fulBucketSize, "expected default bucket size to be the same");
}

void PoolAllocatorTest::XxxTest()
{
	StartTrace(PoolAllocatorTest.XxxTest);
// use the following iterator code sequence if you want to test several things in here
	ROAnything roaCaseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(roaCaseConfig) ) {
		TString strName;
		aEntryIterator.SlotName(strName);
		if ( !strName.Length() ) {
			strName << ":" << aEntryIterator.Index();
		}
		TraceAny(roaCaseConfig, "Running " << strName << " tests with config:");
// enable the following when using AnythingStatisticPolicy to time the test
//		CatchTimeType aTimer(TString("some string to distinguish the different tests at ") << strName, this);
		t_assertm(false, "test me!");
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *PoolAllocatorTest::suite()
{
	StartTrace(PoolAllocatorTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, PoolAllocatorTest, ExcessTrackerEltCtorTest);
	ADD_CASE(testSuite, PoolAllocatorTest, ExcessTrackerEltGetSizeToPowerOf2Test);
	ADD_CASE(testSuite, PoolAllocatorTest, ExcessTrackerEltFindTrackerForSizeTest);
	ADD_CASE(testSuite, PoolAllocatorTest, ExcessTrackerEltInsertTrackerForSizeTest);
	ADD_CASE(testSuite, PoolAllocatorTest, ExcessTrackerEltTest);
//	ADD_CASE(testSuite, PoolAllocatorTest, XxxTest);
// enable the following line if you want the statistics cought to be exported as comma separated values file for analysis in a spreadsheet application
//	ADD_CASE(testSuite, PoolAllocatorTest, ExportCsvStatistics);
	return testSuite;
}
