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
#include "MemHeader.h"

//---- forward declaration -----------------------------------------------
class Allocator;
class MemoryHeader;

//---- StorageTest ----------------------------------------------------------
//!testcases for Storage class
class StorageTest : public TestFramework::TestCase
{
	friend class MemoryHeader;
public:
	//!constructors
	StorageTest(TString tstrName);
	~StorageTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

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
	void DoTimingWith(TString allocatorName, Allocator *gAlloc);
	void SimpleAllocTest(TString allocatorName, Allocator *gAlloc);
	void SimpleSizeHintTest(TString allocatorName, Allocator *gAlloc);
};

#endif
