/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPHeaderRendererTest_H
#define _HTTPHeaderRendererTest_H

#include "TestCase.h"

class HTTPHeaderRendererTest: public TestFramework::TestCase {
public:
	HTTPHeaderRendererTest(TString tname) :
		TestCaseType(tname) {
	}
	//! builds up a suite of testcases for this test
	static Test *suite();
	//! describe this testcase
	void SingleLine();
	//! describe this testcase
	void SingleLineMultiValue();
	//! describe this testcase
	void MultiLine();
	//! describe this testcase
	void WholeHeaderConfig();
};

#endif
