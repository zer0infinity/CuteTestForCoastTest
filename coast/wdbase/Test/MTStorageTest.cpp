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
#include "MTStorageTest.h"

//--- standard modules used ----------------------------------------------------
#include "Timers.h"
#include "PoolAllocator.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//----- AllocTestThread -------------------------------------------------------------
class AllocTestThread : public Thread
{
public:
	// Timings are done in nanoseconds, resolution scales it down to whatever you like
	AllocTestThread(TString allocatorName, Allocator *alloc, MTStorageTest *mts);
	~AllocTestThread() {	}
	void Run();

protected:
//	virtual void DoSignalThreadEnd();

	void DoTimingWith(TString allocatorName, Allocator *alloc);
	TString fAllocatorName;
	MTStorageTest *fTest;

};

//---- MTStorageTest ----------------------------------------------------------------
MTStorageTest::MTStorageTest(TString tname) : TestCase(tname), fFinishedMutex("Finish"), fFinished(0), fStarted(false)
{
#if !defined(__linux__) && !defined(_AIX) && !defined(WIN32)
	thr_setconcurrency(3);
#endif
}

MTStorageTest::~MTStorageTest()
{
}

void MTStorageTest::setUp ()
// setup connector for this TestCase
{
	// setting up a hardcoded debug context
	MT_Storage::Initialize();

} // setUp

void MTStorageTest::tearDown ()
{

} // tearDown

void MTStorageTest::GlobalAllocatorTiming()
{
	fFinished = 0;
	fStarted = false;
	GlobalAllocator ga1;
	GlobalAllocator ga2;
	AllocTestThread alloc1("GlobalAllocator", &ga1, this);
	AllocTestThread alloc2("GlobalAllocator", &ga2, this);
	alloc1.AddObserver(this);
	alloc2.AddObserver(this);
	alloc1.Start();
	alloc2.Start();

	alloc1.CheckState(Thread::eRunning);
	alloc2.CheckState(Thread::eRunning);

	{
		MutexEntry me(fFinishedMutex);
		me.Use();
		fStarted = true;
		fFinishedCond.BroadCast();

	}

	DiffTimer dt;
	dt.Start();
	{
		MutexEntry me(fFinishedMutex);
		me.Use();

		while ( fFinished < 2 ) {
			fFinishedCond.TimedWait(fFinishedMutex, 10);
		}
	}
	StartTrace(MTStorageTest.GlobalAllocatorTiming);
	Trace("GlobalAllocators t: " << dt.Reset());
}

void MTStorageTest::Update(Thread *t, const Anything &args)
{
	StartTrace(MTStorageTest.Update);
	MutexEntry me(fFinishedMutex);
	me.Use();
	long evt = args[0L].AsLong(-1);
	if (evt == Thread::eTerminated) {
		fFinished++;
	}
	fFinishedCond.BroadCast();
}

void MTStorageTest::WaitForStart()
{
	MutexEntry me(fFinishedMutex);
	me.Use();

	while ( !fStarted) {
		fFinishedCond.Wait(fFinishedMutex);
	}
}

void MTStorageTest::PoolAllocatorTiming()
{
	fFinished = 0;
	fStarted = false;

	AllocTestThread alloc1("PoolAllocator", MT_Storage::MakePoolAllocator(), this);
	AllocTestThread alloc2("PoolAllocator", MT_Storage::MakePoolAllocator(), this);

	alloc1.AddObserver(this);
	alloc2.AddObserver(this);

	alloc1.Start();
	alloc2.Start();

	alloc1.CheckState(Thread::eRunning);
	alloc2.CheckState(Thread::eRunning);

	{
		MutexEntry me(fFinishedMutex);
		me.Use();
		fStarted = true;
		fFinishedCond.BroadCast();
	}

	DiffTimer dt;
	dt.Start();

	{
		MutexEntry me(fFinishedMutex);
		me.Use();

		while ( fFinished < 2 ) {
			fFinishedCond.TimedWait(fFinishedMutex, 10);
		}
	}
	StartTrace(MTStorageTest.PoolAllocatorTiming);
	Trace("PoolAllocators t: " << dt.Reset());
}

Test *MTStorageTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	testSuite->addTest (NEW_CASE(MTStorageTest, GlobalAllocatorTiming));
	testSuite->addTest (NEW_CASE(MTStorageTest, PoolAllocatorTiming));
	return testSuite;
}

//---- AllocTestThread ----------------------------------------------------
AllocTestThread::AllocTestThread(TString allocatorName, Allocator *alloc, MTStorageTest *mts)
	: Thread("AllocTestThread", false, true, false, false, alloc), fAllocatorName(allocatorName), fTest(mts)
{
}

void AllocTestThread::Run()
{
	fTest->WaitForStart();
	DoTimingWith(fAllocatorName, fAllocator);
}

void AllocTestThread::DoTimingWith(TString allocatorName, Allocator *myAlloc)
{
	const int cAllocSz = 5;
	long allocSzArr[cAllocSz];
	void *allocPtr[cAllocSz];
	long allocSz = 16;
	const int cRunSize = 10000;

	long i;
	for (i = 0; i < cAllocSz; i++) {
		// generate some alloc sizes
		allocSzArr[i] = allocSz;
		allocSz *= 2;
	}
	for ( long run = cRunSize - 1; run >= 0; run--) {
		if ( run < (cRunSize - cAllocSz) ) {
			myAlloc->Free(allocPtr[(run + cAllocSz) % cAllocSz]);
		}
		allocPtr[run % cAllocSz] = myAlloc->Malloc(allocSzArr[run % cAllocSz]);
	}
	for (i = 0; i < cAllocSz; i++) {
		myAlloc->Free(allocPtr[i]);
	}
}
