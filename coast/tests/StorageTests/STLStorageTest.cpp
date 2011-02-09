/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "STLStorageTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "STLStorage.h"

//--- standard modules used ----------------------------------------------------
#include "PoolAllocator.h"
#include "MemHeader.h"
#include "Dbg.h"

#include <vector>
#include <list>
#include <deque>

//--- c-modules used -----------------------------------------------------------

//---- STLStorageTest ----------------------------------------------------------------
STLStorageTest::STLStorageTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(STLStorageTest.Ctor);
}

STLStorageTest::~STLStorageTest()
{
	StartTrace(STLStorageTest.Dtor);
}

// uncomment if something special needs to be done which isnt already done in base class
//void STLStorageTest::setUp ()
//{
//	StartTrace(STLStorageTest.setUp);
//}
//
//void STLStorageTest::tearDown ()
//{
//	StartTrace(STLStorageTest.tearDown);
//}

void STLStorageTest::GlobalStorageTest()
{
	StartTrace(STLStorageTest.GlobalStorageTest);
	// create a vector, using STLAllocator<> as allocator using Coast::Storage::Global()
	MemChecker aChecker("STLStorageTest.GlobalStorageTest", Coast::Storage::Global());
	{
		std::vector<int, STLStorage::STLAllocator<int> > v;

		// insert elements
		// - causes reallocations
		v.push_back(42);
		v.push_back(56);
		v.push_back(11);
		v.push_back(22);
		v.push_back(33);
		v.push_back(44);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	aChecker.TraceDelta("mem change after test");
}

void STLStorageTest::PoolStorageTest()
{
	StartTrace(STLStorageTest.PoolStorageTest);
	// create a vector, using MyAlloc<> as allocator
	PoolAllocator pa(1, 1024, 12);
	pa.PrintStatistic(3L);
	MemChecker aChecker("STLStorageTest.PoolStorageTest", Coast::Storage::Global());
	{
		STLStorage::STLAllocator<int> stlalloc(&pa);
		std::vector<int, STLStorage::STLAllocator<int> > v(stlalloc);

		// insert elements
		// - causes reallocations
		pa.PrintStatistic(3L);
		v.push_back(42);
		v.push_back(56);
		v.push_back(11);
		v.push_back(22);
		v.push_back(33);
		v.push_back(44);
		pa.PrintStatistic(3L);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	pa.PrintStatistic(3L);
	{
		STLStorage::STLAllocator<int> stlalloc(&pa);
		std::list<int, STLStorage::STLAllocator<int> > v(stlalloc);

		// insert elements
		// - causes reallocations
		pa.PrintStatistic(3L);
		v.push_back(42);
		v.push_back(56);
		v.push_back(11);
		v.push_back(22);
		v.push_back(33);
		v.push_back(44);
		pa.PrintStatistic(3L);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	pa.PrintStatistic(3L);
	{
		STLStorage::STLAllocator<int> stlalloc(&pa);
		std::deque<int, STLStorage::STLAllocator<int> > v(stlalloc);

		// insert elements
		// - causes reallocations
		pa.PrintStatistic(3L);
		v.push_back(42);
		v.push_back(56);
		v.push_back(11);
		v.push_back(22);
		v.push_back(33);
		v.push_back(44);
		pa.PrintStatistic(3L);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	pa.PrintStatistic(3L);
}

struct something {
	int fInt;
	long fLong;
};

void STLStorageTest::AllocatorUsingSMartPtrTest()
{
	StartTrace(STLStorageTest.AllocatorUsingSMartPtrTest);
	typedef something listType;
//	typedef TestStorage::pool_allocator<listType, STLStorage::BoostPoolUserAllocatorCurrent> blaType;
//	typedef STLStorage::pool_allocator<listType, STLStorage::BoostPoolUserAllocatorGlobal> blaType;
	typedef STLStorage::fast_pool_allocator<listType, STLStorage::BoostPoolUserAllocatorGlobal> blaType;
	MemChecker aChecker("STLStorageTest.AllocatorUsingSMartPtrTest", Coast::Storage::Current());
	{
		blaType pool1;
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	Trace("========================== deque tests 1 ==========================");
	{
		std::deque<listType, blaType > v;
		// insert elements
		listType elt1 = { 1, 1 };
		Trace("adding element");
		v.push_back(elt1);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	Trace("========================== list test 1 ==========================");
	{
		std::list<listType, blaType > v;
		// insert elements
		listType elt1 = { 1, 1 };
		Trace("adding element");
		v.push_back(elt1);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	Trace("========================== list test 3 ==========================");
	{
		std::list<listType, blaType > v;
		// insert elements
		listType elt1 = { 1, 1 };
		listType elt2 = { 1, 2 };
		listType elt3 = { 3, 1 };
		Trace("adding element");
		v.push_back(elt1);
		Trace("adding element");
		v.push_back(elt2);
		Trace("adding element");
		v.push_back(elt3);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	Trace("========================== vector tests ==========================");
	{
		std::vector<listType, blaType > v;
		// insert elements
		listType elt1 = { 1, 1 };
		listType elt2 = { 1, 2 };
		listType elt3 = { 3, 1 };
		Trace("adding element");
		v.push_back(elt1);
		Trace("adding element");
		v.push_back(elt2);
		Trace("adding element");
		v.push_back(elt3);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	Trace("========================== deque tests 2 ==========================");
	{
		std::deque<listType, blaType > v;
		// insert elements
		listType elt1 = { 1, 1 };
		listType elt2 = { 1, 2 };
		Trace("adding element");
		v.push_back(elt1);
		Trace("adding element");
		v.push_back(elt2);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	Trace("========================== deque tests 3 ==========================");
	{
		std::deque<listType, blaType > v;
		// insert elements
		listType elt1 = { 1, 1 };
		listType elt2 = { 1, 2 };
		listType elt3 = { 3, 1 };
		Trace("adding element");
		v.push_back(elt1);
		Trace("adding element");
		v.push_back(elt2);
		Trace("adding element");
		v.push_back(elt3);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	Trace("========================== two deque tests 3 ==========================");
	{
		std::deque<listType, blaType > v, w;
		// insert elements
		listType elt1 = { 1, 1 };
		listType elt2 = { 1, 2 };
		listType elt3 = { 3, 1 };
		Trace("adding element");
		v.push_back(elt1);
		w.push_back(elt1);
		Trace("adding element");
		v.push_back(elt2);
		w.push_back(elt2);
		Trace("deleting element");
		v.pop_front();
		w.pop_front();
		Trace("adding element");
		v.push_back(elt3);
		w.push_back(elt3);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
	Trace("========================== two list tests 3 ==========================");
	{
		std::list<listType, blaType > v, w;
		// insert elements
		listType elt1 = { 1, 1 };
		listType elt2 = { 1, 2 };
		listType elt3 = { 3, 1 };
		Trace("adding element");
		v.push_back(elt1);
		w.push_back(elt1);
		Trace("adding element");
		v.push_back(elt2);
		w.push_back(elt2);
		Trace("deleting element");
		v.pop_front();
		w.pop_front();
		Trace("adding element");
		v.push_back(elt3);
		w.push_back(elt3);
	}
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
}

// builds up a suite of testcases, add a line for each testmethod
Test *STLStorageTest::suite ()
{
	StartTrace(STLStorageTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, STLStorageTest, GlobalStorageTest);
	ADD_CASE(testSuite, STLStorageTest, PoolStorageTest);
	ADD_CASE(testSuite, STLStorageTest, AllocatorUsingSMartPtrTest);
	return testSuite;
}
