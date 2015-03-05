/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPFileLoaderTest_H
#define _HTTPFileLoaderTest_H

#include "WDBaseTestPolicies.h"

class HTTPFileLoaderTest: public testframework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	HTTPFileLoaderTest(TString tname) :
		TestCaseType(tname) {
	}
	//!builds up a suite of testcases for this test
	static Test *suite();

	//!test the reply header expansion
	void ReplyHeaderTest();

	//!test the reply header expansion
	void ExecTest();
};

#endif
