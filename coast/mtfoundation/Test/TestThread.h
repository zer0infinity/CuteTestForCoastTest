/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TestThread_H
#define _TestThread_H

#include "ThreadPools.h"

//!utility class - simple Thread to play around - Run returns immediately
class TestThread: public Thread {
public:
	TestThread(const char *name = "TestThread") :
		Thread(name) {
	}
	virtual void Run();
};

//!utility class - simple Thread to play around - Runs until Terminate is called
class TerminateMeTestThread: public TestThread {
public:
	TerminateMeTestThread(bool willStart = true);
	virtual void Run();

protected:
	bool fWillStart;

	virtual bool DoStartRequestedHook(ROAnything args) {
		return fWillStart;
	}
};

//! Thread pool that plays around with TestThreads
class TestThreadPool: public ThreadPoolManager {
public:
	TestThreadPool() :
		ThreadPoolManager("TestThreadPool") {
	}
	bool AllThreadsStarted();
	bool AllThreadsTerminated();

protected:
	//!bottleneck routine to allocate specific thread
	virtual Thread *DoAllocThread(long i, ROAnything args);
};

//! Thread pool that plays around with TerminateMeTestThread
class TerminateTestThreadPool: public TestThreadPool {
public:
	TerminateTestThreadPool() :
		TestThreadPool() {
	}
protected:
	virtual Thread *DoAllocThread(long i, ROAnything args);
};

class TestWorker: public WorkerThread {
public:
	TestWorker(const char *name = "TestWorker");
protected:
	//--- redefine the following virtual methods for your specific workers

	//! used for general initialization of the worker when it is created
	//!  arbitrary parameters may be passed using the ROAnything...
	//!  CAUTION: make sure to copy whatever you need from the ROAnything
	//!           into some instance variable
	virtual void DoInit(ROAnything workerInit);

	//! do the work (using the informations you stored in the instance variables)
	virtual void DoProcessWorkload();
	virtual void DoWorkingHook(ROAnything workloadArgs);

	long fWaitTimeInProcess;
	bool fWasPrepared;
	class WorkerPoolManagerTest *fTest;
};

//! this class demonstrates how to properly subclass WorkerPoolManager
class SamplePoolManager: public WorkerPoolManager {
public:
	SamplePoolManager(const String &name);
	~SamplePoolManager();

protected:
	friend class WorkerPoolManagerTest;

	//!allocate the handle request thread pool
	virtual void DoAllocPool(ROAnything args);

	//!cleanup hook for re-initialization of pool
	virtual void DoDeletePool(ROAnything args);

	virtual WorkerThread *DoGetWorker(long i);
private:
	// block the following default elements of this class
	// because they're not allowed to be used
	SamplePoolManager(const SamplePoolManager &);
	SamplePoolManager &operator=(const SamplePoolManager &);

protected:
	TestWorker *fRequests; // vector storing all request thread objects
};

#endif
