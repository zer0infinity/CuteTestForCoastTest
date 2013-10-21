/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PoolAllocatorTest_H
#define _PoolAllocatorTest_H

#include "FoundationTestTypes.h"

//---- PoolAllocatorTest ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class PoolAllocatorTest : public testframework::TestCaseWithConfig	/* TestCaseWithStatistics | TestCaseWithConfigAndStatistics */
{
public:
	//--- constructors

	/*! \param tstrName name of the test */
	PoolAllocatorTest(TString tstrName);

	//! destroys the test case
	~PoolAllocatorTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

// uncomment if really needed or delete, base class does almost everything
//	//! sets the environment for this test
//	void setUp ();
//
//	//! deletes the environment for this test
//	void tearDown ();
//
//	//! returns name of testcase specific config file name. Default is PoolAllocatorTest.any. Override only if special behavior needed.
//	TString getConfigFileName() { return "PoolAllocatorTestXXX"; }

	//! describe this testcase
	void ExcessTrackerEltCtorTest();
	//! describe this testcase
	void ExcessTrackerEltGetSizeToPowerOf2Test();
	//! describe this testcase
	void ExcessTrackerEltFindTrackerForSizeTest();
	//! describe this testcase
	void ExcessTrackerEltInsertTrackerForSizeTest();
	//! describe this testcase
	void ExcessTrackerEltTest();
	//! test if the allocator prints out still allocated blocks of memory
	void StillUsedBlocksTest();
	//! test if the we can alloc excess memory
	void UseExcessMemTest();
	//! test if larger buckets get split into smaller ones
	void SplitBucketTest();
	//! describe this testcase
	void XxxTest();
};

#endif
