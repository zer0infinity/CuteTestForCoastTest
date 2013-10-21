/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MasterServerTest_H
#define _MasterServerTest_H

#include "WDBaseTestPolicies.h"

class MasterServerTest: public testframework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	MasterServerTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	//!builds up a suite of testcases for this test
	static Test *suite();

	//!sets the environment for this test
	void setUp();

	//! test initialization - run - termination sequences
	void InitRunTerminateTest();

	//! test initialization - run - reset - run - termination sequences
	void InitRunResetRunTerminateTest();

private:
	//! run a test sequence
	void RunTestSequence(ROAnything roaConfig);
};

#endif
