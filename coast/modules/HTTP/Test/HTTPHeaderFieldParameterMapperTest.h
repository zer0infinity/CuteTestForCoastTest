/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPHeaderFieldParameterMapperTest_H
#define _HTTPHeaderFieldParameterMapperTest_H

#include "FoundationTestTypes.h"
#include "WDBaseTestPolicies.h"

class HTTPHeaderFieldParameterMapperTest: public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	//! ConfiguredTestCase constructor
	//! \param name name of the test
	HTTPHeaderFieldParameterMapperTest(TString tstrName) :
		TestCaseType(tstrName) {
	}

	//! builds up a suite of tests
	static Test *suite();

	//! take a simple http response and parse it
	void ConfiguredTests();
};

#endif
