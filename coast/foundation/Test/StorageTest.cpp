/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ITOStorage.h"

//--- interface include --------------------------------------------------------
#include "StorageTest.h"

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "DiffTimer.h"
#include "PoolAllocator.h"

//--- c-library modules used ---------------------------------------------------

//---- StorageTest ----------------------------------------------------------------
StorageTest::StorageTest(TString tname) : TestCaseType(tname)
{
}

StorageTest::~StorageTest()
{
}

void StorageTest::SimpleCallocSetsToZeroTest()
{
	GlobalAllocator ga;
	const long sz = 3000;
	char *ptr[sz];
	long i = 0;
	for ( i = 0 ; i < sz ; i++ ) {
		ptr[i] = (char *)ga.Malloc(i + 1); // add one otherwise we do not alloc something in the first step
		*ptr[i] = '\xA5';
		assertEqual('\xA5', *ptr[i]);
	}
	for ( i = 0 ; i < sz ; i++ ) {
		ga.Free(ptr[i]);
	}
	for ( i = 0 ; i < sz ; i++ ) {
		ptr[i] = (char *)ga.Calloc(i + 1, 1); // add one otherwise we do not alloc something in the first step
		TString msg("i = ");
		msg << i;
		assertEqualm((long)'\0', (long)*ptr[i], msg);
	}
	for ( i = 0 ; i < sz ; i++ ) {
		ga.Free(ptr[i]);
	}
}

void StorageTest::LeakTest()
{
	// watch the sequence of declaration
	MemChecker mc("Test Global", Storage::Global());
	String str;
	{
		StartTraceMem(StorageTest.LeakTest);
		str = "Test a leak";
	}
	//PS: we now have a different measurement
	//assertEqual(12, mc.CheckDelta());
	t_assertm(mc.CheckDelta() >= 12, "memory leaked as expected");
}

void StorageTest::SimpleAllocTest(TString allocatorName, Allocator *pAllocator)
{
	MemChecker mc(allocatorName, pAllocator);
	const long sz = 3000;
	char *ptr[sz];
	long i = 0;
	for ( i = 0 ; i < sz ; i++ ) {
		ptr[i] = (char *)pAllocator->Malloc(i);
	}
	assertEqual(4498500, mc.CheckDelta());
	for ( i = 0 ; i < sz ; i++ ) {
		pAllocator->Free(ptr[i]);
	}
	pAllocator->PrintStatistic();
	assertEqual(0, mc.CheckDelta());
}
void StorageTest::SimpleSizeHintTest(TString allocatorName, Allocator *pAllocator)
{
	const size_t sz = 3000;
	size_t i = 0;
	for ( i = 0 ; i < sz ; i++ ) {
		t_assert(pAllocator->SizeHint(i) >= i);
	}
}

void StorageTest::PoolSizeHintTest()
{
	PoolAllocator pa(1);
	SimpleSizeHintTest("Poolallocator", &pa);
	const size_t maxbucketsize = 0x2000;
	// test some significant values
	// must be adjusted if implementation of pool allocator is changed
	assertEqual(16, pa.SizeHint(1));
	assertEqual(16, pa.SizeHint(16));
	assertEqual(32, pa.SizeHint(17));
	assertEqual(32, pa.SizeHint(17));
	assertEqual(0x2000, pa.SizeHint(0x1fff));
	assertEqual(0x2fff, pa.SizeHint(0x2fff));
	assertEqual(32, pa.SizeHint(32));
	assertEqual(16, pa.SizeHint(0));
	{
		for (size_t s = 0, expected = 16; s < 0x7fff; s += 8 ) {
			if (expected < s) {
				expected *= 2;    // buckets double in size
			}
			if (expected <= maxbucketsize) {
				assertEqual(expected, pa.SizeHint(s));
			} else {
				assertEqual(s, pa.SizeHint(s));
			}

		}
	}
	{
		for (size_t s = 1, expected = 16; s < 0x7fff; s += 8 ) {
			if (expected < s) {
				expected *= 2;    // buckets double in size
			}
			if (expected <= maxbucketsize) {
				assertEqual(expected, pa.SizeHint(s));
			} else {
				assertEqual(s, pa.SizeHint(s));
			}
		}
	}
}

void StorageTest::AllocatorTest()
{
	GlobalAllocator ga;
	SimpleSizeHintTest("GlobalAllocator",  &ga);
	SimpleAllocTest("GlobalAllocator",  &ga);
}

void StorageTest::AllocatorTiming()
{
	PoolAllocator pa(1, 8 * 1024, 21);
	DoTimingWith("PoolAllocator", &pa);
	GlobalAllocator ga;
	DoTimingWith("GlobalAllocator", &ga);
}

void StorageTest::DoTimingWith(TString allocatorName, Allocator *pAllocator)
{
	{
		StartTraceMem1(StorageTest.DoTimingWith, pAllocator);
		MemChecker mc("StorageTest.DoTimingWith", pAllocator);
		const int cAllocSz = 5;
		long allocSzArr[cAllocSz];
		void *allocPtr[cAllocSz];
		long allocSz = 16;
		const int cRunSize = 10000;
		l_long t;
		long i;
		for (i = 0; i < cAllocSz; i++) {
			// generate some alloc sizes
			allocSzArr[i] = allocSz;
			allocSz *= 2;
		}

		DiffTimer dt;
		dt.Start();
		for ( long lrun = cRunSize - 1; lrun >= 0; lrun--) {
			if ( lrun < (cRunSize - cAllocSz) ) {
				pAllocator->Free(allocPtr[(lrun + cAllocSz) % cAllocSz]);
			}
			allocPtr[lrun % cAllocSz] = pAllocator->Malloc(allocSzArr[lrun % cAllocSz]);
		}
		for (i = 0; i < cAllocSz; i++) {
			pAllocator->Free(allocPtr[i]);
		}
		t = dt.Reset();
		pAllocator->PrintStatistic();
		assertEqual(0, mc.CheckDelta());
	}
}

void StorageTest::AnyStorageGlobalAssignment()
{
	// test assignment with Storage::Global
	Anything a;
	Anything b;

	t_assert(a.GetAllocator() == Storage::Global());
	t_assert(b.GetAllocator() == Storage::Global());

	a = b;
	t_assert(a.GetAllocator() == Storage::Global());
	t_assert(b.GetAllocator() == Storage::Global());

	b = 1;
	t_assert(b.GetAllocator() == Storage::Global());
	a = b;
	t_assert(a.GetAllocator() == Storage::Global());
	t_assert(b.GetAllocator() == Storage::Global());
}

void StorageTest::AnyEqualAllocatorAssignment()
{
	GlobalAllocator ga;
	Anything a(&ga);
	Anything b(&ga);

	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &ga);

	a = b;
	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &ga);

	b = 1;
	t_assert(b.GetAllocator() == &ga);
	a = b;
	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &ga);

}

void StorageTest::AnyEqualPoolAllocatorAssignment()
{
	PoolAllocator pa(100, 10);
	Anything a(&pa);
	Anything b(&pa);

	t_assert(a.GetAllocator() == &pa);
	t_assert(b.GetAllocator() == &pa);

	a = b;
	t_assert(a.GetAllocator() == &pa);
	t_assert(b.GetAllocator() == &pa);

	b = 1;
	t_assert(b.GetAllocator() == &pa);
	a = b;
	t_assert(a.GetAllocator() == &pa);
	t_assert(b.GetAllocator() == &pa);

}

void StorageTest::AnyDifferentAllocatorAssignment()
{
	GlobalAllocator ga;
	PoolAllocator pa(100, 10);
	Anything a(&ga);
	Anything b(&pa);

	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &pa);

	a = b;
	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &pa);

	b = 1;
	t_assert(b.GetAllocator() == &pa);

	a = b;
	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &pa);

	b = a;
	t_assert(a.GetAllocator() == &ga);
	t_assert(b.GetAllocator() == &pa);
}

void StorageTest::AnyPathology()
{
	//try to force a null allocator with SetAllocator
	Anything a;
	t_assert(a.GetAllocator() == Storage::Global());
	t_assert(!a.SetAllocator(0));
	t_assert(a.GetAllocator() == Storage::Global());
	a = "what a fool";
	t_assert(Storage::Global() == a.GetAllocator());
	assertEqual("what a fool", a.AsCharPtr());

	//try to force a null allocator in constructor
	Anything a1((Allocator *)0);
	t_assert(a1.GetAllocator() == Storage::Global());
	a1 = "what a fool";
	t_assert(Storage::Global() == a1.GetAllocator());
	assertEqual("what a fool", a1.AsCharPtr());
}

void StorageTest::AnyAssignment()
{
	AnyStorageGlobalAssignment();
	AnyEqualAllocatorAssignment();
	AnyEqualPoolAllocatorAssignment();
	AnyDifferentAllocatorAssignment();
	AnyPathology();
}

Test *StorageTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, StorageTest, PoolSizeHintTest);
	ADD_CASE(testSuite, StorageTest, SimpleCallocSetsToZeroTest);
	ADD_CASE(testSuite, StorageTest, AllocatorTest);
	ADD_CASE(testSuite, StorageTest, AllocatorTiming);
	ADD_CASE(testSuite, StorageTest, LeakTest);
	ADD_CASE(testSuite, StorageTest, AnyAssignment);
	return testSuite;
}
