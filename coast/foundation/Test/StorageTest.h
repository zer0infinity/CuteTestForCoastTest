/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StorageTest_H
#define _StorageTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
class EXPORTDECL_FOUNDATION Allocator;
class EXPORTDECL_FOUNDATION GlobalAllocator;

//---- StorageTest ----------------------------------------------------------
//!testcases for Storage class
class StorageTest : public TestCase
{
public:
	//!constructors
	StorageTest(TString tstrName);
	~StorageTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();
	//!tests the Socket class with a fd from Connector
	void PoolSizeHintTest();
	void AllocatorTiming();
	void AllocatorTest();
	void SimpleCallocSetsToZeroTest();
	void LeakTest();
	void AnyAssignment();
	void AnyEqualAllocatorAssignment();
	void AnyStorageGlobalAssignment();
	void AnyEqualPoolAllocatorAssignment();
	void AnyDifferentAllocatorAssignment();
	void AnyPathology();

protected:
	//--- subclass api

	void DoTimingWith(TString allocatorName, Allocator *gAlloc);
	void SimpleAllocTest(TString allocatorName, Allocator *gAlloc);
	void SimpleSizeHintTest(TString allocatorName, Allocator *gAlloc);

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

#endif
