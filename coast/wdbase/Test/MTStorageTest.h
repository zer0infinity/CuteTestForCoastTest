/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MTStorageTest_H
#define _MTStorageTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "Threads.h"

//---- forward declaration -----------------------------------------------
class Allocator;
class PoolAllocator;
class GlobalAllocator;

//---- MTStorageTest ----------------------------------------------------------
//!Test driver for tracing facility
class MTStorageTest : public TestCase, ThreadObserver
{
public:
	//!constructors
	MTStorageTest(TString name);
	~MTStorageTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	void WaitForStart();
	virtual void Update(Thread *t, const Anything &args);
protected:
	//--- subclass api

	//!tests the StorageAllocators
	void GlobalAllocatorTiming();
	void PoolAllocatorTiming();

	Mutex fFinishedMutex;
	Condition fFinishedCond;
	long fFinished;
	bool fStarted;

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

#endif
