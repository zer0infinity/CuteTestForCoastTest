/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPProcessorWithChecksTest_H
#define _HTTPProcessorWithChecksTest_H

#include "WDBaseTestPolicies.h"

class HTTPProcessorWithChecksTest: public testframework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	//! TestCase constructor
	//! \param name name of the test
	HTTPProcessorWithChecksTest(TString tstrName);

	//! builds up a suite of testcases for this test
	static Test *suite();

	TString getConfigFileName();

	//! describe this testcase
	void CheckFaultyHeadersTest();
};

#endif
