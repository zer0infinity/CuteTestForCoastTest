/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ObjectList_rTest_H
#define _ObjectList_rTest_H

#include "TestCase.h"

class ObjectList_rTest: public TestFramework::TestCase {
public:
	ObjectList_rTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	//! builds up a suite of testcases for this test
	static Test *suite();
	//! describe this testcase
	void CtorTest();
};

#endif
