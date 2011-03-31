/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TimeStampTestThread_H
#define _TimeStampTestThread_H

#include "ThreadPools.h"

//---- forward declaration -----------------------------------------------
//---- TestWorker -----------------------------------------------
class TestWorker : public WorkerThread
{
public:
	TestWorker();
	~TestWorker();

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

	long fNumberOfRuns;
	long fWaitTimeInProcess;
	bool fWasPrepared, fCompare, fUtcCtor;

	class ThreadedTimeStampTest *fTest;
};

//---- SamplePoolManager ------------------------------------------------
//! this class demonstrates how to properly subclass WorkerPoolManager
class SamplePoolManager : public WorkerPoolManager
{
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
	TestWorker *fRequests;				// vector storing all request thread objects
};

#endif
