/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestThread.h"

//--- module under test --------------------------------------------------------
#include "WorkerPoolManagerTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- TestThread ----------------------------------------------------------------
TestThread::TestThread(const char *name)
	: Thread(name)
{
	StartTrace(TestThread.TestThread);
}

void TestThread::Run()
{
	SetWorking();
	StartTrace(TestThread.Run);
	SetReady();
}

//--- TerminateMeTestThread -------------------------------------------
TerminateMeTestThread::TerminateMeTestThread(bool willStart)
	: TestThread("TerminateMeTestThread")
	, fWillStart(willStart)
{
}

void TerminateMeTestThread::Run()
{
	StartTrace(TerminateMeTestThread.Run);
	CheckState(eTerminationRequested);
}

//--- TestThreadPool -------------------------------------------
TestThreadPool::TestThreadPool()
	: ThreadPoolManager("TestThreadPool")
{
	StartTrace(TestThreadPool.TestThreadPool);
}

TestThreadPool::~TestThreadPool()
{
	StartTrace(TestThreadPool.~TestThreadPool);
}

Thread *TestThreadPool::DoAllocThread(long i, ROAnything args)
{
	StartTrace(TestThreadPool.DoAllocThread);
	return new TestThread;
}

bool TestThreadPool::AllThreadsStarted()
{
	StartTrace(TestThreadPool.AllThreadsStarted);
	return (fStartedThreads == GetPoolSize());
}

bool TestThreadPool::AllThreadsTerminated()
{
	StartTrace(TestThreadPool.AllThreadsTerminated);
	for (long i = 0; i < GetPoolSize(); i++) {
		Thread *tt = DoGetThread(i);
		if (! (tt->GetState() == Thread::eTerminated) ) {
			return false;
		}
	}
	return true;
}

//--- TerminateTestThreadPool -------------------------------------------
TerminateTestThreadPool::TerminateTestThreadPool()
	: TestThreadPool()
{
}

Thread *TerminateTestThreadPool::DoAllocThread(long i, ROAnything args)
{
	return new TerminateMeTestThread;
}

//---- TestWorker -----------------------------------------------
TestWorker::TestWorker(const char *name)
	: WorkerThread(name)
	, fWaitTimeInProcess(2)
	, fWasPrepared(false)
{
}

TestWorker::~TestWorker()
{
}

void TestWorker::DoInit(ROAnything workerInit)
{
	StartTrace(TestWorker.DoInit);
	fWaitTimeInProcess = workerInit["timeout"].AsLong(2);
	fTest = (WorkerPoolManagerTest *)workerInit["test"].AsIFAObject(0);
}

void TestWorker::DoWorkingHook(ROAnything workloadArgs)
{
	StartTrace(TestWorker.DoWorkingHook);
	fWasPrepared = true;
}

void TestWorker::DoProcessWorkload()
{
	StartTrace(TestWorker.DoProcessWorkload);
	Thread::Wait(fWaitTimeInProcess);
	if ( CheckRunningState( eWorking ) ) {
		fTest->CheckProcessWorkload(true, fWasPrepared);
	}
	fWasPrepared = false;
}

//---- SamplePoolManager ------------------------------------------------
SamplePoolManager::SamplePoolManager(const String &name)
	: WorkerPoolManager(name)
	, fRequests(0)
{
}

SamplePoolManager::~SamplePoolManager()
{
	Terminate();
	Anything dummy;
	DoDeletePool(dummy);
}

void SamplePoolManager::DoAllocPool(ROAnything args)
{
	// create the pool of worker threads
	fRequests = new TestWorker[GetPoolSize()];
}

WorkerThread *SamplePoolManager::DoGetWorker(long i)
{
	// accessor for one specific worker
	if (fRequests) {
		return &(fRequests[i]);
	}
	return 0;
}

void SamplePoolManager::DoDeletePool(ROAnything args)
{
	// cleanup of the sub-class specific stuff
	// CAUTION: this cleanup method may be called repeatedly..
	if (fRequests) {
		delete [ ] fRequests;
		fRequests = 0;
	}
}
