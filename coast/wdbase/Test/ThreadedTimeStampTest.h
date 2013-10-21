/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ThreadedTimeStampTest_H
#define _ThreadedTimeStampTest_H

#include "FoundationTestTypes.h"
#include "Threads.h"
class Mutex;

class ThreadedTimeStampTest : public testframework::TestCaseWithConfigAndStatistics
{
public:
	//!TestCase constructor
	//! \param name name of the test
	ThreadedTimeStampTest(TString tstrName);

	//!destroys the test case
	~ThreadedTimeStampTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	TString getConfigFileName();

	//!test initialization
	void TimestampConcurrencyTest();

	void CheckProcessWorkload(bool isReady, bool wasPrepared);
	void CheckNumberOfRuns(long numberOfRuns, long doneRuns, String threadName);

protected:
	// guards asserts
	void DoTimeStampConcurrencyTest(long numberOfRuns, long numberOfThreads, long concurrencyFactor, ROAnything roaConfig);
	Mutex fCheckMutex;
};

#endif
