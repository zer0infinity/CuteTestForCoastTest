/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Threads.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TString.h"
#include "TestLocation.h"
#include "TestLocList.h"
#include "TestResult.h"

//--- module under test --------------------------------------------------------
#include "ThreadedTimeStampTest.h"
#include "TimeStamp.h"

//--- interface include --------------------------------------------------------
#include "TimeStampTestThread.h"

//---- TestWorker -----------------------------------------------
TestWorker::TestWorker()
	: WorkerThread()
	, fWaitTimeInProcess(0)
	, fWasPrepared(false)
	, fCompare(false)
	, fUtcCtor(false)
{
}

TestWorker::~TestWorker()
{
}

void TestWorker::DoInit(ROAnything workerInit)
{
	StartTrace(TestWorker.DoInit);
	fNumberOfRuns = workerInit["NumberOfRuns"].AsLong(1);
	fWaitTimeInProcess = workerInit["timeout"].AsLong(0);
	fCompare = workerInit["CompareStamps"].AsBool(false);
	fUtcCtor = workerInit["UTCCtorTest"].AsBool(false);
	fTest = (ThreadedTimeStampTest *)workerInit["test"].AsIFAObject(0);
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
	CheckRunningState(eWorking);
	bool bSomething = false;
	long l;
	for (l = 0; l < fNumberOfRuns; l++) {
		if ( fUtcCtor ) {
			TimeStamp aStamp1(946782245);
			if ( fCompare ) {
				TimeStamp aStamp2(946782246);
				if ( aStamp2 > aStamp1 ) {
					bSomething = true;
				}
			}
		} else {
			TimeStamp aStamp1("20000102030405");
			if ( fCompare ) {
				TimeStamp aStamp2("20000102030406");
				if ( aStamp2 > aStamp1 ) {
					bSomething = true;
				}
			}
		}
	}
	fTest->CheckNumberOfRuns(fNumberOfRuns, l, GetName());

	if ( CheckRunningState( eWorking ) ) {
		fTest->CheckProcessWorkload(true, fWasPrepared);
	}
	fWasPrepared = false;
}

//---- SamplePoolManager ------------------------------------------------
SamplePoolManager::SamplePoolManager(const String &name) : WorkerPoolManager(name), fRequests(0) {}

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
