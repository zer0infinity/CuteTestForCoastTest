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
class SegStoreAllocatorTest: public testframework::TestCase {
public:
	SegStoreAllocatorTest(TString tname) :
		TestCaseType(tname) {
	}
	static Test *suite();
	void CreateSimpleAllocator();
	void RealAnythingTest();
};

#endif		//not defined _SegStoreAllocatorTest_H
