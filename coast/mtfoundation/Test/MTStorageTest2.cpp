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
#include "MT_Storage.h"

//--- interface include --------------------------------------------------------
#include "MTStorageTest2.h"

//--- standard modules used ----------------------------------------------------
#include "Threads.h"
#include "DiffTimer.h"
#include "PoolAllocator.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <io.h>
#endif

//--- TestWorkerThread --------------------------------------------------
class TestWorkerThread : public Thread
{
protected:
	TestWorkerThread(Allocator *a);
	~TestWorkerThread();
protected:
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
	: TestWorkerThread(a), fData(a)
{
//	fName.SetAllocator(a);		// use local pool so as not to disturb measurements
//	fName= "DataProviderThread";
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
MTStorageTest2::MTStorageTest2(TString tname) : TestCase(tname), fGlobal(0), fPool(0)
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
	MT_Storage::Initialize();
	if (!fGlobal) {
		fGlobal = Storage::Global();
	}
	if (!fPool) {
		fPool = MT_Storage::MakePoolAllocator(2000, 25);
		MT_Storage::RefAllocator(fPool);
	}
}

void MTStorageTest2::tearDown()
{
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
#ifdef MEM_DEBUG
//	l_long l= fGlobal->CurrentlyAllocated();
	assertEqualm(0, fPool->CurrentlyAllocated(), "expected fPool to be empty");

#endif
	DataProviderThread *t1 = new DataProviderThread(fPool);
	t1->Start(fPool);

	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

	// everything should have been allocated within pool!
#ifdef MEM_DEBUG
	t_assert( fPool->CurrentlyAllocated() > 0);
//	t_assert( fGlobal->CurrentlyAllocated() == l); no longer true
#endif
	delete t1;
#ifdef MEM_DEBUG
	assertEqualm(0, fPool->CurrentlyAllocated(), "expected fPool to be empty");
#endif
}

void MTStorageTest2::twoThreadAssignmentTest()
{
	StartTrace1(MTStorageTest2.twoThreadAssignmentTest, "ThrdId: " << Thread::MyId());
#ifdef MEM_DEBUG
	l_long l = fGlobal->CurrentlyAllocated();
#endif
	DataProviderThread *t1 = new DataProviderThread(fPool);
	t1->Start(fPool);

	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

#ifdef MEM_DEBUG
//	t_assert( fGlobal->CurrentlyAllocated() == l); no longer true
#endif

	Anything y = t1->GetData();
	delete t1;

	// data should have been copied to global store now

#ifdef MEM_DEBUG
	assertEqualm(0, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	t_assert( fGlobal->CurrentlyAllocated() > l);
#endif
}

void MTStorageTest2::twoThreadCopyConstructorTest()
{
	StartTrace1(MTStorageTest2.twoThreadCopyConstructorTest, "ThrdId: " << Thread::MyId());
#ifdef MEM_DEBUG
	l_long l = fGlobal->CurrentlyAllocated();
#endif
	DataProviderThread *t1 = new DataProviderThread(fPool);
	t1->Start(fPool);

	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

#ifdef MEM_DEBUG
//	t_assert( fGlobal->CurrentlyAllocated() == l); no longer true
#endif
	Anything y(t1->GetData());		// should be equivalent to the use of operator=
	delete t1;

	// data should have been copied to global store now

#ifdef MEM_DEBUG
	assertEqualm(0, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	t_assert( fGlobal->CurrentlyAllocated() > l);
#endif
}

void MTStorageTest2::twoThreadArrayAccessTest()
{
	StartTrace1(MTStorageTest2.twoThreadArrayAccessTest, "ThrdId: " << Thread::MyId());
#ifdef MEM_DEBUG
	l_long l = fGlobal->CurrentlyAllocated();
#endif
	{
		DataProviderThread *t1 = new DataProviderThread(fPool);
		t1->Start(fPool);

		// wait for other thread to finish
		t1->CheckState(Thread::eTerminated);

		ROAnything sub(t1->GetData()["Sub"]["2"]);	// no copy should be necessary for access

		// CAUTION: always pass a reference to long lived Anything to ROAnything! methods
		//          often return a temporary Anything which is *NOT* suitable to initialize
		//          a ROAnything...

#ifdef MEM_DEBUG
		t_assert( fGlobal->CurrentlyAllocated() == l); //no longer true
#endif
		assertEqual( "ok", sub.AsCharPtr("") );

		Anything copy(t1->GetData()["Sub"]["2"]);	// must be copied since allocators dont match
#ifdef MEM_DEBUG
		t_assert( fGlobal->CurrentlyAllocated() > l);
#endif
		// data should have been copied to global store now
		delete t1;
	}
#ifdef MEM_DEBUG
	// new we should be back where we started
	assertEqualm(0, fPool->CurrentlyAllocated(), "expected fPool to be empty");
	t_assert( fGlobal->CurrentlyAllocated() == l); //no longer true
#endif
}

void MTStorageTest2::reusePoolTest()
{
	StartTrace1(MTStorageTest2.reusePoolTest, "ThrdId: " << Thread::MyId());

#ifdef MEM_DEBUG
//	l_long l= fGlobal->CurrentlyAllocated();
#endif

	Allocator *pa = MT_Storage::MakePoolAllocator(3);
	t_assertm(pa != 0, "expected allocation of PoolAllocator to succeed");
	if (!pa) {
		return;
	}
	MT_Storage::RefAllocator(pa);	// need to refcount poolstorage
	t_assertm(pa->RefCnt() == 1, "expected refcnt to be 1");
	DataProviderThread *t1 = new DataProviderThread(pa);
	t_assertm(pa->RefCnt() == 2, "expected refcnt to be 2");

	// do some work
	t1->Start(pa);

	// wait for thread to finish
	t1->CheckState(Thread::eTerminated);

	t_assertm(pa->RefCnt() == 2, "expected refcnt to be 2");
	delete t1;
	t_assertm(pa->RefCnt() == 1, "expected refcnt to be 1");

	// do it again
	t1 = new DataProviderThread(pa);
	t_assertm(pa->RefCnt() == 2, "expected refcnt to be 2");

	// do some work
	t1->Start(pa);

	// wait for other thread to finish
	t1->CheckState(Thread::eTerminated);

	t_assertm(pa->RefCnt() == 2, "expected refcnt to be 2");
	delete t1;
	t_assertm(pa->RefCnt() == 1, "expected refcnt to be 1");

	MT_Storage::UnrefAllocator(pa);	// need to refcount poolstorage

#ifdef MEM_DEBUG
	// new we should be back where we started
//	t_assert( fGlobal->CurrentlyAllocated() == l); no longer true
#endif

}

Test *MTStorageTest2::suite()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;
	testSuite->addTest (NEW_CASE(MTStorageTest2, trivialTest));
	testSuite->addTest (NEW_CASE(MTStorageTest2, twoThreadTest));
	testSuite->addTest (NEW_CASE(MTStorageTest2, twoThreadAssignmentTest));
	testSuite->addTest (NEW_CASE(MTStorageTest2, twoThreadCopyConstructorTest));
	testSuite->addTest (NEW_CASE(MTStorageTest2, twoThreadArrayAccessTest));
	testSuite->addTest (NEW_CASE(MTStorageTest2, reusePoolTest));
	return testSuite;
}
