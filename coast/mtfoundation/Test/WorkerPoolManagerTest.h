/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WorkerPoolManagerTest_H
#define _WorkerPoolManagerTest_H

#include "TestCase.h"
#include "Threads.h"

class WorkerPoolManagerTest: public TestFramework::TestCase {
public:
	WorkerPoolManagerTest(TString tname) :
		TestCaseType(tname), fCheckMutex("WorkerPoolManager") {
	}
	//!builds up a suite of testcases for this test
	static Test *suite();

	//!test initialization
	void InitTest();

	//!test EnterAndLeaves
	void EnterLeaveTests();

	void CheckProcessWorkload(bool isReady, bool wasPrepared);

protected:
	// guards asserts
	Mutex fCheckMutex;
};

#endif
