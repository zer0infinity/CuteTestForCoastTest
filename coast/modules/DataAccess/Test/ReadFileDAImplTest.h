/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ReadFileDAImplTest_H
#define _ReadFileDAImplTest_H

#include "TestCase.h"

class ReadFileDAImplTest: public testframework::TestCase {
public:
	ReadFileDAImplTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	//! builds up a suite of testcases for this test
	static Test *suite();
	//! describe this testcase
	void GetFileStreamTest();
	void GetFileNameTest();
};

#endif
