/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SystemAPITest_H
#define _SystemAPITest_H

#include "TestCase.h"

struct condthreadparam;
class SystemAPITest: public TestFramework::TestCase {
public:
	SystemAPITest(TString tname) :
		TestCaseType(tname) {
	}
	//!builds up a suite of testcases for this test
	static Test *suite();

protected:
	void PrintResult(Anything result);
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
