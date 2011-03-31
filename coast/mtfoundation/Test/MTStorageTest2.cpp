/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "MT_Storage.h"
#include "MTStorageTest2.h"
#include "Threads.h"
#include "DiffTimer.h"
#include "PoolAllocator.h"
#include "Dbg.h"
#if defined(WIN32)
#include <io.h>
#endif

//--- TestWorkerThread --------------------------------------------------
class TestWorkerThread : public Thread
{
protected:
	TestWorkerThread(Allocator *a);
	virtual ~TestWorkerThread();
};

TestWorkerThread::TestWorkerThread(Allocator *a)
	: Thread("TestWorkerThread", false, true, false, false, a)
{
}

TestWorkerThread::~TestWorkerThread()
{
}

//--- DataProviderThread --------------------------------------------------

class DataProviderThread : public TestWorkerThread
{
public:
	DataProviderThread(Allocator *a);

	Anything &GetData() {
		return fData;
	}

protected:
	virtual void Run();
	Anything fData;
};

DataProviderThread::DataProviderThread(Allocator *a)
	: TestWorkerThread(a), fData(Coast::Storage::Global())
{
}

void DataProviderThread::Run()
{
	fData.Append("sfdsfdsfd");
	fData.Append(1432);
	fData.Append(2323.4343);

	Anything b;
	b["1"] = 1;
	b["2"] = "ok";
	b["3"] = "hhgfjhgfjhgf";
	b["4"] = 0.4334;

	fData["Sub"] = b;
}

//---- MTStorageTest2 ----------------------------------------------------------------
MTStorageTest2::MTStorageTest2(TString tname) : TestCaseType(tname), fGlobal(0), fPool(0)
{
	StartTrace1(MTStorageTest2.Ctor, "ThrdId: " << Thread::MyId());
}

MTStorageTest2::~MTStorageTest2()
{
	StartTrace1(MTStorageTest2.Dtor, "ThrdId: " << Thread::MyId());
}

void MTStorageTest2::setUp()
{
	StartTrace1(MTStorageTest2.setUp, "ThrdId: " << Thread::MyId());
	// setting up a hardcoded debug context
	if (!fGlobal) {
		fGlobal = Coast::Storage::Global();
	}
	if (!fPool) {
		fPool = MT_Storage::MakePoolAllocator(2000, 25);
		MT_Storage::RefAllocator(fPool);
	}
}

void MTStorageTest2::tearDown()
{
	StartTrace(MTStorageTest2.tearDown);
	MT_Storage::UnrefAllocator(fPool);
	fPool = 0;
}

void MTStorageTest2::trivialTest()
{
	StartTrace(MTStorageTest2.trivialTest);
	t_assert( fGlobal != fPool );	// allocators should be different
}

void MTStorageTest2::twoThreadTest()
{
	StartTrace1(MTStorageTest2.twoThreadTest, "ThrdId: " << Thread::MyId());
	assertEqualm(0, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	DataProviderThread *t1 = new (Coast::Storage::Global()) DataProviderThread(fPool);
	t1->Start(fPool);

	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

	// everything should have been allocated within pool!
	// the current implementation allows size testing, eg. tracking of allocated and freed memory only in Coast::Storage::GetStatisticLevel() >= 1
	if ( Coast::Storage::GetStatisticLevel() >= 1 ) {
		assertComparem( fPool->CurrentlyAllocated(), greater, 0LL, "everything should have been allocated within pool XXX: changes when semantic of Thread::eStarted changes!");
	}
	delete t1;
	assertComparem(0LL, equal_to, fPool->CurrentlyAllocated(), "expected fPool to be empty");
}

void MTStorageTest2::twoThreadAssignmentTest()
{
	StartTrace1(MTStorageTest2.twoThreadAssignmentTest, "ThrdId: " << Thread::MyId());
	l_long l = fGlobal->CurrentlyAllocated();
	DataProviderThread *t1 = new (Coast::Storage::Global()) DataProviderThread(fPool);
	t1->Start(fPool);

	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

	Anything y = t1->GetData();
	delete t1;

	// data should have been copied to global store now

	assertComparem(0LL, equal_to, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	// the current implementation allows size testing, eg. tracking of allocated and freed memory only in Coast::Storage::GetStatisticLevel() >= 1
	if ( Coast::Storage::GetStatisticLevel() >= 1 ) {
		assertCompare( fGlobal->CurrentlyAllocated(), greater, l);
	}
}

void MTStorageTest2::twoThreadCopyConstructorTest()
{
	StartTrace1(MTStorageTest2.twoThreadCopyConstructorTest, "ThrdId: " << Thread::MyId());
	l_long l = fGlobal->CurrentlyAllocated();
	DataProviderThread *t1 = new (Coast::Storage::Global()) DataProviderThread(fPool);
	t1->Start(fPool);

	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

	Anything y(t1->GetData());		// should be equivalent to the use of operator=
	delete t1;

	// data should have been copied to global store now

	assertComparem(0LL, equal_to, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	// the current implementation allows size testing, eg. tracking of allocated and freed memory only in Coast::Storage::GetStatisticLevel() >= 1
	if ( Coast::Storage::GetStatisticLevel() >= 1 ) {
		assertCompare( fGlobal->CurrentlyAllocated(), greater, l);
	}
}

void MTStorageTest2::twoThreadArrayAccessTest()
{
	StartTrace1(MTStorageTest2.twoThreadArrayAccessTest, "ThrdId: " << Thread::MyId());
	DataProviderThread *t1 = new (Coast::Storage::Global()) DataProviderThread(fPool);
	l_long l = fGlobal->CurrentlyAllocated();
	{
		t1->Start(fPool);
		assertCompare( l, equal_to, fGlobal->CurrentlyAllocated());

		// wait for other thread to finish
		t1->CheckState(Thread::eTerminated);
		assertCompare( l, equal_to, fGlobal->CurrentlyAllocated());
		// CAUTION: always pass a reference to long lived Anything to ROAnything! methods
		//          often return a temporary Anything which is *NOT* suitable to initialize
		//          a ROAnything...
		ROAnything sub(t1->GetData()["Sub"]["2"]);	// no copy should be necessary for access
		assertEqual( "ok", sub.AsCharPtr("") );

		Anything copy(t1->GetData()["Sub"]["2"]);	// must be copied since allocators dont match
		// the current implementation allows size testing, eg. tracking of allocated and freed memory only in Coast::Storage::GetStatisticLevel() >= 1
		if ( Coast::Storage::GetStatisticLevel() >= 1 ) {
			assertCompare( fGlobal->CurrentlyAllocated(), greater, l);
		}
		// data should have been copied to global store now
		delete t1;
	}
	// new we should be back where we started
	assertComparem(0LL, equal_to, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	// the current implementation allows size testing, eg. tracking of allocated and freed memory only in Coast::Storage::GetStatisticLevel() >= 1
	if ( Coast::Storage::GetStatisticLevel() >= 1 ) {
		assertCompare( fGlobal->CurrentlyAllocated(), equal_to, l);
	}
}

void MTStorageTest2::reusePoolTest()
{
	StartTrace1(MTStorageTest2.reusePoolTest, "ThrdId: " << Thread::MyId());

	Allocator *pa = MT_Storage::MakePoolAllocator(3);
	t_assertm(pa != 0, "expected allocation of PoolAllocator to succeed");
	if (!pa) {
		return;
	}
	MT_Storage::RefAllocator(pa);	// need to refcount poolstorage
	assertEqualm(1L, pa->RefCnt(), "expected refcnt to be 1");
	DataProviderThread *t1 = new (Coast::Storage::Global()) DataProviderThread(pa);
	assertEqualm(2L, pa->RefCnt(), "expected refcnt to be 2");

	// do some work
	t1->Start(pa);

	// wait for thread to finish
	t1->CheckState(Thread::eTerminated);
	assertEqualm(2L, pa->RefCnt(), "expected refcnt to be 2");
	delete t1;
	assertEqualm(1L, pa->RefCnt(), "expected refcnt to be 1");

	// do it again
	t1 = new (Coast::Storage::Global()) DataProviderThread(pa);
	assertEqualm(2L, pa->RefCnt(), "expected refcnt to be 2");

	// do some work
	t1->Start(pa);
	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

	assertEqualm(2L, pa->RefCnt(), "expected refcnt to be 2");
	delete t1;
	assertEqualm(1L, pa->RefCnt(), "expected refcnt to be 1");

	MT_Storage::UnrefAllocator(pa);	// need to refcount poolstorage
}

Test *MTStorageTest2::suite()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MTStorageTest2, trivialTest);
	ADD_CASE(testSuite, MTStorageTest2, twoThreadTest);
	ADD_CASE(testSuite, MTStorageTest2, twoThreadAssignmentTest);
	ADD_CASE(testSuite, MTStorageTest2, twoThreadCopyConstructorTest);
	ADD_CASE(testSuite, MTStorageTest2, twoThreadArrayAccessTest);
	ADD_CASE(testSuite, MTStorageTest2, reusePoolTest);
	return testSuite;
}
