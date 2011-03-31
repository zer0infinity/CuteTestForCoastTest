/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MTStorageTest2_H
#define _MTStorageTest2_H

#include "TestCase.h"

class Allocator;
class PoolAllocator;
class GlobalAllocator;

//---- MTStorageTest2 ----------------------------------------------------------
//!Test driver for tracing facility
class MTStorageTest2 : public TestFramework::TestCase
{
public:
	//!constructors
	MTStorageTest2(TString tstrName);
	~MTStorageTest2();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//--- subclass api

	void trivialTest();
	void twoThreadTest();
	void twoThreadAssignmentTest();
	void twoThreadCopyConstructorTest();
	void twoThreadArrayAccessTest();
	void reusePoolTest();

private:
	Allocator *fGlobal, *fPool;
};

#endif
