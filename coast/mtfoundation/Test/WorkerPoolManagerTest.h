/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WorkerPoolManagerTest_H
#define _WorkerPoolManagerTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

class Mutex;

//---- WorkerPoolManagerTest ----------------------------------------------------------
//!TestCases description
class WorkerPoolManagerTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	WorkerPoolManagerTest(TString name);

	//!destroys the test case
	~WorkerPoolManagerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!test initialization
	void InitTest();

	//!test EnterAndLeaves
	void EnterLeaveTests();

	void CheckProcessWorkload(bool isReady, bool wasPrepared);
	void CheckPrepare2Run(bool isWorking, bool wasPrepared);
protected:
	// guards asserts
	Mutex fCheckMutex;

};

#endif
