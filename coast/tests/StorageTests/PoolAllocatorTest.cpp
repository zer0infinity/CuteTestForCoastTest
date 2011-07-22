/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "PoolAllocatorTest.h"
#include "PoolAllocator.h"
#include "TestSuite.h"
#include "Tracer.h"
#include "MemHeader.h"

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

void PoolAllocatorTest::ExcessTrackerEltCtorTest()
{
	StartTrace(PoolAllocatorTest.ExcessTrackerEltCtorTest);
	ExcessTrackerElt aDfltCtor;
	t_assertm(aDfltCtor.fpTracker == NULL, "expected pointer to tracker to be NULL");
	t_assertm(aDfltCtor.fpNext == NULL, "expected pointer to next element to be NULL");
	assertEqualm(0, aDfltCtor.fulPayloadSize, "expected default bucket size to be 0");
	ExcessTrackerElt aParamCtor((MemTracker *)0x12345, (ExcessTrackerElt *)0xabcde, 33);
	t_assertm(aParamCtor.fpTracker == (MemTracker *)0x12345, "expected pointer to be equal");
	t_assertm(aParamCtor.fpNext == (ExcessTrackerElt *)0xabcde, "expected pointer to next element to be equal");
	assertEqualm(33, aParamCtor.fulPayloadSize, "expected default bucket size to be the same");
	ExcessTrackerElt aCopyCtor(aParamCtor);
	t_assertm(aCopyCtor.fpTracker == (MemTracker *)0x12345, "expected pointer to be equal");
	t_assertm(aCopyCtor.fpNext == (ExcessTrackerElt *)0xabcde, "expected pointer to next element to be equal");
	assertEqualm(33, aCopyCtor.fulPayloadSize, "expected default bucket size to be the same");
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
	u_long ulMinPayloadSz = 16UL;
	ExcessTrackerElt *pElt128 = new ExcessTrackerElt((MemTracker *)128, NULL, ulMinPayloadSz << 3);
	ExcessTrackerElt *pElt64 = new ExcessTrackerElt((MemTracker *)64, pElt128, ulMinPayloadSz << 2);
	ExcessTrackerElt *pElt32 = new ExcessTrackerElt((MemTracker *)32, pElt64, ulMinPayloadSz << 1);
	t_assertm(pElt128->fpNext == 0, "expected pointer to be equal");
	assertEqualm(0, (long)pElt32->FindTrackerForSize(ulMinPayloadSz), "expected no tracker to be found because the smallest tracker[16] was not available");
	assertEqualm(32, (long)pElt32->FindTrackerForSize(ulMinPayloadSz + 1), "expected correct tracker to be found");
	assertEqualm(32, (long)pElt32->FindTrackerForSize(ulMinPayloadSz << 1), "expected correct tracker to be found");
	assertEqualm(64, (long)pElt32->FindTrackerForSize((ulMinPayloadSz << 1) + 1), "expected correct tracker to be found");
	assertEqualm(64, (long)pElt32->FindTrackerForSize(ulMinPayloadSz << 2), "expected correct tracker to be found");
	assertEqualm(128, (long)pElt32->FindTrackerForSize(ulMinPayloadSz << 3), "expected correct tracker to be found");
	assertEqualm(0, (long)pElt32->FindTrackerForSize((ulMinPayloadSz << 3) + 100), "expected correct tracker to be found");
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
	assertEqualm(32, aRoot.fulPayloadSize, "expected default bucket size to be the same");
	ExcessTrackerElt *pElt32 = &aRoot, *pElt64 = NULL, *pElt48 = NULL, *pElt16 = NULL;
	if ( t_assertm( ( pElt64 = aRoot.InsertTrackerForSize((MemTracker *)64, 64)) != NULL, "expected valid new element") ) {
		t_assertm( pElt64 != &aRoot, "expected new element not to be the root element");
		t_assertm(pElt64->fpTracker == (MemTracker *)64, "expected pointer to be equal");
		t_assertm(pElt64->fpNext == NULL, "expected pointer to next element to be equal");
		assertEqualm(64, pElt64->fulPayloadSize, "expected default bucket size to be the same");
		t_assertm(aRoot.fpTracker == (MemTracker *)32, "expected pointer to be equal");
		t_assertm(aRoot.fpNext == pElt64, "expected pointer to next element to be equal");
		assertEqualm(32, aRoot.fulPayloadSize, "expected default bucket size to be the same");
		if ( t_assertm( ( pElt48 = aRoot.InsertTrackerForSize((MemTracker *)48, 48)) != NULL, "expected valid new element") ) {
			t_assertm( pElt48 != &aRoot, "expected new element not to be the root element");
			t_assertm( pElt48 != pElt64, "expected new element not to be the root element");
			t_assertm(pElt48->fpTracker == (MemTracker *)48, "expected pointer to be equal");
			t_assertm(pElt48->fpNext == pElt64, "expected pointer to next element to be valid");
			assertEqualm(48, pElt48->fulPayloadSize, "expected default bucket size to be the same");
			t_assertm(aRoot.fpTracker == (MemTracker *)32, "expected pointer to be equal");
			t_assertm(aRoot.fpNext == pElt48, "expected pointer to next element to be equal");
			assertEqualm(32, aRoot.fulPayloadSize, "expected default bucket size to be the same");
			if ( t_assertm( ( pElt16 = aRoot.InsertTrackerForSize((MemTracker *)16, 16)) != NULL, "expected valid new element") ) {
				t_assertm( pElt16 == &aRoot, "expected new element not to be the root element");
				t_assertm(pElt16->fpTracker == (MemTracker *)16, "expected pointer to be equal");
				assertEqualm(16, pElt16->fulPayloadSize, "expected default bucket size to be the same");
				t_assertm(aRoot.fpTracker == (MemTracker *)16, "expected pointer to be equal");
				assertEqualm(16, aRoot.fulPayloadSize, "expected default bucket size to be the same");
				pElt32 = pElt16->fpNext;
				t_assertm(pElt32->fpTracker == (MemTracker *)32, "expected pointer to be equal");
				assertEqualm(32, pElt32->fulPayloadSize, "expected default bucket size to be the same");
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
	ExcessTrackerElt aRoot, *pElt64 = NULL, *pElt32 = NULL;
	MemTracker *pTrack32 = aRoot[32];
	Trace("trackername32 [" << pTrack32->GetName() << "]");
	t_assertm(aRoot.fpTracker == pTrack32, "expected pointer to be equal");
	t_assertm(aRoot.fpNext == NULL, "expected pointer to next element to be equal");
	assertEqualm(32, aRoot.fulPayloadSize, "expected default bucket size to be the same");
	MemTracker *pTrack64 = aRoot[64];
	Trace("trackername64 [" << pTrack64->GetName() << "]");
	t_assertm(aRoot.fpTracker == pTrack32, "expected pointer to be equal");
	assertEqualm(32, aRoot.fulPayloadSize, "expected default bucket size to be the same");
	pElt64 = aRoot.fpNext;
	t_assertm(pElt64->fpTracker == pTrack64, "expected pointer to be equal");
	assertEqualm(64, pElt64->fulPayloadSize, "expected default bucket size to be the same");
	MemTracker *pTrack16 = aRoot[16];
	Trace("trackername16 [" << pTrack16->GetName() << "]");
	t_assertm(aRoot.fpTracker == pTrack16, "expected pointer to be equal");
	assertEqualm(16, aRoot.fulPayloadSize, "expected default bucket size to be the same");
	pElt32 = aRoot.fpNext;
	t_assertm(pElt32->fpTracker == pTrack32, "expected pointer to be equal");
	assertEqualm(32, pElt32->fulPayloadSize, "expected default bucket size to be the same");
	t_assertm(pElt32->fpNext == pElt64, "expected pointer to next element to be equal");
	t_assertm(aRoot.fpTracker == pTrack16, "expected pointer to be equal");
	t_assertm(aRoot.fpNext == pElt32, "expected pointer to next element to be equal");
	assertEqualm(16, aRoot.fulPayloadSize, "expected default bucket size to be the same");
}

void PoolAllocatorTest::StillUsedBlocksTest()
{
	StartTrace(PoolAllocatorTest.StillUsedBlocksTest);
	// the current implementation allows size testing, eg. tracking of allocated and freed memory only in Coast::Storage::GetStatisticLevel() >= 1
	if ( Coast::Storage::GetStatisticLevel() >= 1 ) {
		PoolAllocator pa(1, 1024, 4);
		// this alloc should allocate a block of size 16 + Coast::Memory::AlignedSize<MemoryHeader>::value
		void *p16 = pa.Calloc(1, 16);
		// CurrentlyAllocated() counts only usable size, usable should be 16bytes
		assertCompare(static_cast<ul_long>(16), equal_to, pa.CurrentlyAllocated());
		void *p32 = pa.Calloc(1, 32);
		// CurrentlyAllocated() counts only usable size, usable should be 32bytes
		assertCompare(static_cast<ul_long>(48), equal_to, pa.CurrentlyAllocated());
		pa.DumpStillAllocated();
		pa.Free(p32);
		pa.Free(p16);
	}
}

void PoolAllocatorTest::UseExcessMemTest()
{
	StartTrace(PoolAllocatorTest.UseExcessMemTest);

	// allocate only 1024 byte ( 1 * 1024 )
	// use bucket sizes up to 512 bytes (num=6)
	PoolAllocator pa(9, 1, 6);
	// this alloc should allocate a block of size 512 + Coast::Memory::AlignedSize<MemoryHeader>::value in pool
	void *p512 = pa.Calloc(1, 512);
	MemoryHeader *pH512 = pa.RealMemStart(p512);
	assertCompare(MemoryHeader::eUsed, equal_to, pH512->fState);

	// this alloc should allocate a block of size 512 + Coast::Memory::AlignedSize<MemoryHeader>::value in excess space
	// -> because ( 1024 - (512+AlignedSize()) ) < (512+AlignedSize())
	void *pE512 = pa.Calloc(1, 512);
	MemoryHeader *pHE512 = pa.RealMemStart(pE512);
	assertCompare(MemoryHeader::eUsedNotPooled, equal_to, pHE512->fState);

	pa.Free(pE512);
	pa.Free(p512);
	assertCompare(MemoryHeader::eFree, equal_to, pH512->fState);
}

void PoolAllocatorTest::SplitBucketTest()
{
	StartTrace(PoolAllocatorTest.SplitBucketTest);

	// allocate only 1024 byte ( 1 * 1024 )
	// use bucket sizes up to 512 bytes (num=6)
	PoolAllocator pa(9, 1, 6);
	// this alloc should allocate a block of size 512 + Coast::Memory::AlignedSize<MemoryHeader>::value in pool
	void *p512 = pa.Calloc(1, 512);
	MemoryHeader *pH512 = pa.RealMemStart(p512);
	assertCompare(MemoryHeader::eUsed, equal_to, pH512->fState);
	pa.Free(p512);
	assertCompare(MemoryHeader::eFree, equal_to, pH512->fState);

	void *p256 = pa.Calloc(1, 256);
	MemoryHeader *pH256 = pa.RealMemStart(p256);
	assertCompare(MemoryHeader::eUsed, equal_to, pH256->fState);

	// the next Calloc should fail finding enough memory in the pool
	// because ( 1024 - (512+AlignedSize()) - (256+AlignedSize()) ) < (256+AlignedSize())
	// -> but as a 512 byte block is free, it should use its memory to create a 256byte block out of it
	void *p256X = pa.Calloc(1, 256);
	MemoryHeader *pH256X = pa.RealMemStart(p256X);
	assertCompare(MemoryHeader::eUsed, equal_to, pH256X->fState);

	// this alloc should now allocate a block of size 512 + Coast::Memory::AlignedSize<MemoryHeader>::value in excess space
	// because the previously available unused block was reused as 256byte block and no mor pool memory is available
	void *pE512 = pa.Calloc(1, 512);
	MemoryHeader *pHE512 = pa.RealMemStart(pE512);
	assertCompare(MemoryHeader::eUsedNotPooled, equal_to, pHE512->fState);

	pa.Free(pE512);
	pa.Free(p256X);
	pa.Free(p256);
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
	ADD_CASE(testSuite, PoolAllocatorTest, StillUsedBlocksTest);
	ADD_CASE(testSuite, PoolAllocatorTest, UseExcessMemTest);
	ADD_CASE(testSuite, PoolAllocatorTest, SplitBucketTest);
//	ADD_CASE(testSuite, PoolAllocatorTest, XxxTest);
// enable the following line if you want the statistics cought to be exported as comma separated values file for analysis in a spreadsheet application
//	ADD_CASE(testSuite, PoolAllocatorTest, ExportCsvStatistics);
	return testSuite;
}
