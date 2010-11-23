/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WorkerPoolManagerModulePoolManager_H
#define _WorkerPoolManagerModulePoolManager_H

//---- Thread include -------------------------------------------------
#include "ThreadPools.h"

//---- WorkerPoolManagerModuleWorker -----------------------------------------------
class WorkerPoolManagerModuleWorker : public WorkerThread
{
public:
	WorkerPoolManagerModuleWorker(const char *name = "WorkerPoolManagerModuleWorker");
	~WorkerPoolManagerModuleWorker();

protected:
	//--- redefine the following virtual methods for your specific workers

	//: used for general initialization of the worker when it is created
	//  arbitrary parameters may be passed using the ROAnything...
	//  CAUTION: make sure to copy whatever you need from the ROAnything
	//           into some instance variable
	virtual void DoInit(ROAnything workerInit);

	//: do the work (using the informations you stored in the instance variables)
	virtual void DoProcessWorkload();

	//:subclass hook
	virtual void DoWorkingHook(ROAnything workerConfig);
	virtual void DoTerminationRequestHook(ROAnything);
	virtual void DoTerminatedHook();

private:
	Anything	fWork;
	Anything	fWorkerInitCfg;
};

//---- SybCTPoolManager ------------------------------------------------
// this class demonstrates how to properly subclass WorkerPoolManager

class WorkerPoolManagerModulePoolManager : public WorkerPoolManager
{
public:
	WorkerPoolManagerModulePoolManager(String name);
	virtual ~WorkerPoolManagerModulePoolManager();

	bool Init(const ROAnything config);
	void Enter(Anything &args);

protected:
//	friend WorkerPoolManagerTest;

	//:allocate the handle request thread pool
	virtual void DoAllocPool(ROAnything args);

	//:cleanup hook for re-initialization of pool
	virtual void DoDeletePool(ROAnything args);

	virtual WorkerThread *DoGetWorker(long i);

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	WorkerPoolManagerModulePoolManager(const WorkerPoolManagerModulePoolManager &);
	WorkerPoolManagerModulePoolManager &operator=(const WorkerPoolManagerModulePoolManager &);

protected:
	WorkerPoolManagerModuleWorker *fRequests;				// vector storing all request thread objects
};

#endif
