/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTPoolManager_H
#define _SybCTPoolManager_H

//---- Thread include -------------------------------------------------
#include "config_sybasect.h"
#include "ThreadPools.h"
#include "SybCT.h"

//---- SybaseWorker -----------------------------------------------
class EXPORTDECL_SYBASECT SybaseWorker : public WorkerThread
{
public:
	SybaseWorker(const char *name = "SybaseWorker");
	~SybaseWorker();

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
	SybCT		*fpCT;
	Anything	fWork;
};

//---- SybCTPoolManager ------------------------------------------------
// this class demonstrates how to properly subclass WorkerPoolManager

class EXPORTDECL_SYBASECT SybCTPoolManager : public WorkerPoolManager
{
public:
	SybCTPoolManager(String name);
	virtual ~SybCTPoolManager();

	bool Init(const ROAnything config);
	void Work(Anything &args);

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
	SybCTPoolManager(const SybCTPoolManager &);
	SybCTPoolManager &operator=(const SybCTPoolManager &);

protected:
	SybaseWorker *fRequests;				// vector storing all request thread objects
	CS_CONTEXT *fpContext;
	Anything fContextMessages;
};

#endif
