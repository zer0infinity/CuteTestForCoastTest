/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MTStorageTest_H
#define _MTStorageTest_H

#include "TestCase.h"
#include "Threads.h"

//---- MTStorageTest ----------------------------------------------------------
//!Test driver for tracing facility
class MTStorageTest: public TestFramework::TestCase, public Observable<Thread, ROAnything>::Observer {
	typedef Observable<Thread, ROAnything> tBaseClass;
	typedef tBaseClass::tObservedPtr tObservedPtr;
	typedef tBaseClass::tArgsRef tArgsRef;
public:
	//!constructors
	MTStorageTest(TString tstrName);
	//!builds up a suite of testcases for this test
	static Test *suite();

	void WaitForStart();
	virtual void Update(tObservedPtr pObserved, tArgsRef roaUpdateArgs);
protected:
	//--- subclass api

	//!tests the StorageAllocators
	void GlobalAllocatorTiming();
	void PoolAllocatorTiming();

	Mutex fFinishedMutex;
	Condition fFinishedCond;
	long fFinished;
	bool fStarted;
};

#endif
