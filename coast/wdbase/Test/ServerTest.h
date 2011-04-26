/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServerTest_H
#define _ServerTest_H

#include "WDBaseTestPolicies.h"

class ServerTest: public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	ServerTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();

	//!sets the environment for this test
	void setUp();

	//! test initialization - run - termination sequences
	void InitRunTerminateAcceptorTest();

	//! test initialization - run - termination sequences
	void InitRunTerminateLeaderFollowerTest();

	//! test initialization - run - reset - run - termination sequences
	void InitRunResetRunTerminateAcceptorTest();

	//! test initialization - run - reset - run - termination sequences
	void InitRunResetRunTerminateLeaderFollowerTest();

	//! run a test sequence
	void RunTestSequence();
};

#endif
