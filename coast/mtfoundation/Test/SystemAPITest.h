/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SystemAPITest_H
#define _SystemAPITest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- SystemAPITest ----------------------------------------------------------
//!TestCases description
struct condthreadparam;
class SystemAPITest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	SystemAPITest(TString tstrName);

	//!destroys the test case
	~SystemAPITest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

protected:
	void PrintResult(Anything result );
	void MUTEXTest();
	void SEMATest();
	void CONDITIONSignalSingleTest();
	void CONDITIONSignalManyTest();
	void CONDITIONBroadcastSingleTest();
	void CONDITIONBroadcastManyTest();
	void CONDITIONTimedWaitTimeoutTest();
	void CONDITIONSTestMain(int nofthreads, bool broadcast);
	void SleepAndSignal(condthreadparam &p, int expectedcount, bool broadcast);
};

#endif
