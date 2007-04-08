/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MemTrackerTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ITOStorage.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "MemHeader.h"

//--- c-modules used -----------------------------------------------------------

//---- MemTrackerTest ----------------------------------------------------------------
MemTrackerTest::MemTrackerTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(MemTrackerTest.Ctor);
}

MemTrackerTest::~MemTrackerTest()
{
	StartTrace(MemTrackerTest.Dtor);
}

// uncomment if something special needs to be done which isnt already done in base class
//void MemTrackerTest::setUp ()
//{
//	StartTrace(MemTrackerTest.setUp);
//}
//
//void MemTrackerTest::tearDown ()
//{
//	StartTrace(MemTrackerTest.tearDown);
//}

void MemTrackerTest::TrackAllocFreeTest()
{
	StartTrace(MemTrackerTest.TrackAllocFreeTest);
	MemTracker aTracker("AllocFreeTracker");

	void *vp32 = ::calloc(1, (MemoryHeader::AlignedSize() + 16));
	void *vp48 = ::calloc(1, (MemoryHeader::AlignedSize() + 32));

	memset(vp32, 0x55, (MemoryHeader::AlignedSize() + 16));
	memset(vp48, 0xaa, (MemoryHeader::AlignedSize() + 32));

	// use placement new operator to allocate MemoryHeader from
	MemoryHeader *pMH16 = new(vp32) MemoryHeader(16, MemoryHeader::eUsed), *pMH32 = new(vp48) MemoryHeader(32, MemoryHeader::eUsed);

	assertCompare(aTracker.fAllocated, equal_to, 0LL);
	assertCompare(aTracker.fMaxAllocated, equal_to, 0LL);
	assertCompare(aTracker.fNumAllocs, equal_to, 0LL);
	assertCompare(aTracker.fNumFrees, equal_to, 0LL);
	assertCompare(aTracker.fSizeAllocated, equal_to, 0LL);
	assertCompare(aTracker.fSizeFreed, equal_to, 0LL);
	if ( aTracker.fpUsedList ) {
		assertCompare(aTracker.fpUsedList->size(), equal_to, (size_t)0);
	}
	aTracker.TrackAlloc(pMH16);
	assertCompare(aTracker.fAllocated, equal_to, 16LL);
	assertCompare(aTracker.fMaxAllocated, equal_to, 16LL);
	assertCompare(aTracker.fNumAllocs, equal_to, 1LL);
	assertCompare(aTracker.fNumFrees, equal_to, 0LL);
	assertCompare(aTracker.fSizeAllocated, equal_to, 16LL);
	assertCompare(aTracker.fSizeFreed, equal_to, 0LL);
	if ( aTracker.fpUsedList ) {
		assertCompare(aTracker.fpUsedList->size(), equal_to, (size_t)1);
	}
	aTracker.TrackAlloc(pMH32);
	assertCompare(aTracker.fAllocated, equal_to, 48LL);
	assertCompare(aTracker.fMaxAllocated, equal_to, 48LL);
	assertCompare(aTracker.fNumAllocs, equal_to, 2LL);
	assertCompare(aTracker.fNumFrees, equal_to, 0LL);
	assertCompare(aTracker.fSizeAllocated, equal_to, 48LL);
	assertCompare(aTracker.fSizeFreed, equal_to, 0LL);
	if ( aTracker.fpUsedList ) {
		assertCompare(aTracker.fpUsedList->size(), equal_to, (size_t)2);
	}
	aTracker.TrackFree(pMH16);
	assertCompare(aTracker.fAllocated, equal_to, 32LL);
	assertCompare(aTracker.fMaxAllocated, equal_to, 48LL);
	assertCompare(aTracker.fNumAllocs, equal_to, 2LL);
	assertCompare(aTracker.fNumFrees, equal_to, 1LL);
	assertCompare(aTracker.fSizeAllocated, equal_to, 48LL);
	assertCompare(aTracker.fSizeFreed, equal_to, 16LL);
	if ( aTracker.fpUsedList ) {
		assertCompare(aTracker.fpUsedList->size(), equal_to, (size_t)1);
	}

	aTracker.TrackFree(pMH32);
	assertCompare(aTracker.fAllocated, equal_to, 0LL);
	assertCompare(aTracker.fMaxAllocated, equal_to, 48LL);
	assertCompare(aTracker.fNumAllocs, equal_to, 2LL);
	assertCompare(aTracker.fNumFrees, equal_to, 2LL);
	assertCompare(aTracker.fSizeAllocated, equal_to, 48LL);
	assertCompare(aTracker.fSizeFreed, equal_to, 48LL);
	if ( aTracker.fpUsedList ) {
		assertCompare(aTracker.fpUsedList->size(), equal_to, (size_t)0);
	}
}

void MemTrackerTest::XxxTest()
{
	StartTrace(MemTrackerTest.XxxTest);
}

// builds up a suite of testcases, add a line for each testmethod
Test *MemTrackerTest::suite ()
{
	StartTrace(MemTrackerTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MemTrackerTest, TrackAllocFreeTest);
//	ADD_CASE(testSuite, MemTrackerTest, XxxTest);
	return testSuite;
}
