/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#ifndef _SegStoreAllocatorTest_H
#define _SegStoreAllocatorTest_H

#include "TestCase.h"//lint !e537

//---- SegStoreAllocatorTest ----------------------------------------------------------
//!TestCases for the SegStoreAllocatorTest class
class SegStoreAllocatorTest : public TestFramework::TestCase
{
public:
	//--- constructors
	SegStoreAllocatorTest(TString tstrName);

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	// Testcases
	void CreateSimpleAllocator();
};

#endif		//not defined _SegStoreAllocatorTest_H
