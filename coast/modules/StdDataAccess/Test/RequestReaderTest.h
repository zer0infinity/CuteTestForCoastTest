/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestReaderTest_H
#define _RequestReaderTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"

//---- RequestReaderTest ----------------------------------------------------
class RequestReaderTest: public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	RequestReaderTest(TString tstrName);

	//! destroys the test case
	~RequestReaderTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite();

	TString getConfigFileName();

	//! describe this testcase
	void ReadMinimalInputTest();
	void CleanupRequestLineTest();
};

#endif
