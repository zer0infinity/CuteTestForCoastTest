/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPRequestReaderTest_H
#define _HTTPRequestReaderTest_H

#include "WDBaseTestPolicies.h"

class HTTPRequestReaderTest: public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	HTTPRequestReaderTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	//! builds up a suite of testcases for this test
	static Test *suite();
	TString getConfigFileName() {
		return "HTTPRequestReaderTestConfig";
	}
	//! describe this testcase
	void ReadMinimalInputTest();
};

#endif
