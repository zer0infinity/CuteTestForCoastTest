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
	// create a vector, using MyAlloc<> as allocator
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

void STLStorageTest::PoolStorageTest()
{
	StartTrace(STLStorageTest.PoolStorageTest);
	// create a vector, using MyAlloc<> as allocator
	PoolAllocator pa(1, 1024, 12);
	pa.PrintStatistic(3L);
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
	pa.PrintStatistic(3L);
}
// builds up a suite of testcases, add a line for each testmethod
Test *STLStorageTest::suite ()
{
	StartTrace(STLStorageTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, STLStorageTest, GlobalStorageTest);
	ADD_CASE(testSuite, STLStorageTest, PoolStorageTest);
	return testSuite;
}
