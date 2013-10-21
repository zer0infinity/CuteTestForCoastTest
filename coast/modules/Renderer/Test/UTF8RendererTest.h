/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _UTF8RendererTest_H
#define _UTF8RendererTest_H

#include "TestCase.h"

class UTF8RendererTest: public testframework::TestCase {
public:
	UTF8RendererTest(TString tname) :
		TestCaseType(tname) {
	}
	//! builds up a suite of testcases for this test
	static Test *suite();

	//! describe this testcase
	void conversionTest();
};

#endif
