/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MemTrackerTest.h"
#include "TestSuite.h"
#include "ITOStorage.h"
#include "Tracer.h"
#include "MemHeader.h"

MemTrackerTest::MemTrackerTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(MemTrackerTest.Ctor);
}

void MemTrackerTest::TrackAllocFreeTest() {
	StartTrace(MemTrackerTest.TrackAllocFreeTest);
	MemTracker aTracker("AllocFreeTracker");
	const size_t alignedSize = coast::memory::AlignedSize<MemoryHeader>::value;

	void *vp32 = ::calloc(1, (alignedSize + 16));
	void *vp48 = ::calloc(1, (alignedSize + 32));

	memset(vp32, 0x55, (alignedSize + 16));
	memset(vp48, 0xaa, (alignedSize + 32));

	// use placement new operator to allocate MemoryHeader from
	MemoryHeader *pMH16 = new (vp32) MemoryHeader(16, MemoryHeader::eUsed), *pMH32 = new (vp48) MemoryHeader(32, MemoryHeader::eUsed);

	assertCompare(static_cast<l_long>(0), equal_to, aTracker.fAllocated);
	assertCompare(static_cast<l_long>(0), equal_to, aTracker.fMaxAllocated);
	assertCompare(static_cast<ul_long>(0), equal_to, aTracker.fNumAllocs);
	assertCompare(static_cast<ul_long>(0), equal_to, aTracker.fNumFrees);
	assertCompare(static_cast<ul_long>(0), equal_to, aTracker.fSizeAllocated);
	assertCompare(static_cast<ul_long>(0), equal_to, aTracker.fSizeFreed);
	if (aTracker.fpUsedList) {
		assertCompare(static_cast<size_t>(0), equal_to, aTracker.fpUsedList->size());
	}
	aTracker.TrackAlloc(pMH16);
	assertCompare(static_cast<l_long>(16), equal_to, aTracker.fAllocated);
	assertCompare(static_cast<l_long>(16), equal_to, aTracker.fMaxAllocated);
	assertCompare(static_cast<ul_long>(1), equal_to, aTracker.fNumAllocs);
	assertCompare(static_cast<ul_long>(0), equal_to, aTracker.fNumFrees);
	assertCompare(static_cast<ul_long>(16), equal_to, aTracker.fSizeAllocated);
	assertCompare(static_cast<ul_long>(0), equal_to, aTracker.fSizeFreed);
	if (aTracker.fpUsedList) {
		assertCompare(static_cast<size_t>(1), equal_to, aTracker.fpUsedList->size());
	}
	aTracker.TrackAlloc(pMH32);
	assertCompare(static_cast<l_long>(48), equal_to, aTracker.fAllocated);
	assertCompare(static_cast<l_long>(48), equal_to, aTracker.fMaxAllocated);
	assertCompare(static_cast<ul_long>(2), equal_to, aTracker.fNumAllocs);
	assertCompare(static_cast<ul_long>(0), equal_to, aTracker.fNumFrees);
	assertCompare(static_cast<ul_long>(48), equal_to, aTracker.fSizeAllocated);
	assertCompare(static_cast<ul_long>(0), equal_to, aTracker.fSizeFreed);
	if (aTracker.fpUsedList) {
		assertCompare(static_cast<size_t>(2), equal_to, aTracker.fpUsedList->size());
	}
	aTracker.TrackFree(pMH16);
	assertCompare(static_cast<l_long>(32), equal_to, aTracker.fAllocated);
	assertCompare(static_cast<l_long>(48), equal_to, aTracker.fMaxAllocated);
	assertCompare(static_cast<ul_long>(2), equal_to, aTracker.fNumAllocs);
	assertCompare(static_cast<ul_long>(1), equal_to, aTracker.fNumFrees);
	assertCompare(static_cast<ul_long>(48), equal_to, aTracker.fSizeAllocated);
	assertCompare(static_cast<ul_long>(16), equal_to, aTracker.fSizeFreed);
	if (aTracker.fpUsedList) {
		assertCompare(static_cast<size_t>(1), equal_to, aTracker.fpUsedList->size());
	}

	aTracker.TrackFree(pMH32);
	assertCompare(static_cast<l_long>(0), equal_to, aTracker.fAllocated);
	assertCompare(static_cast<l_long>(48), equal_to, aTracker.fMaxAllocated);
	assertCompare(static_cast<ul_long>(2), equal_to, aTracker.fNumAllocs);
	assertCompare(static_cast<ul_long>(2), equal_to, aTracker.fNumFrees);
	assertCompare(static_cast<ul_long>(48), equal_to, aTracker.fSizeAllocated);
	assertCompare(static_cast<ul_long>(48), equal_to, aTracker.fSizeFreed);
	if (aTracker.fpUsedList) {
		assertCompare(static_cast<size_t>(0), equal_to, aTracker.fpUsedList->size());
	}
}

Test *MemTrackerTest::suite() {
	StartTrace(MemTrackerTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MemTrackerTest, TrackAllocFreeTest);
	return testSuite;
}
