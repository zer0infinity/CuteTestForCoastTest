/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WorkerPoolManagerModule_H
#define _WorkerPoolManagerModule_H

//---- WDModule include -------------------------------------------------
#include "WDModule.h"
#include "ThreadPools.h"
#include "config_workerpoolmanagermodule.h"
class WorkerPoolManagerModulePoolManager;

//---- WorkerPoolManagerModule ----------------------------------------------------------
//: comment action
//	Structure of config:
//<PRE>	{
//		/WorkerPoolManagerModule
//		{
//			/Pools
//			{
//				/PoolOne
//				{
//					/Workers <long>
//				}
//				/PoolN
//				{
//					/Workers <long>
//				}
//		}
//      ...
//	}</PRE>
class EXPORTDECL_WORKERPOOLMANAGERMODULE WorkerPoolManagerModule : public WDModule
{
public:
	//--- constructors
	WorkerPoolManagerModule(const char *name);
	~WorkerPoolManagerModule();

	//:implementers should initialize module using config
	virtual bool Init(const Anything &config);
	//:implementers should terminate module expecting destruction
	virtual bool Finis();

	// function to get workerpool for a server
	WorkerPoolManagerModulePoolManager *GetPoolManager(const char *pPoolName);

//    //:initializes module after termination for reinitialization; default uses Init; check if this applies
//    virtual bool ResetInit(const Anything &config);
//    //:terminates module for reinitialization; default uses Finis; check if this applies
//    virtual bool ResetFinis(const Anything &config);

private:
	Anything	fWorkerPools;
};

#endif
