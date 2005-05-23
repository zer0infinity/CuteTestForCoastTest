/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingPerfTest_H
#define _AnythingPerfTest_H

//---- baseclass include -------------------------------------------------
#include "StatisticTestCase.h"
#include "Anything.h"

class AnythingPerfTest;
typedef void (AnythingPerfTest::* LoopFunctor)(const char *pName, const Anything &a, const long iterations);

//---- AnythingPerfTest ----------------------------------------------------------
//!TestCases description
class AnythingPerfTest : public StatisticTestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	AnythingPerfTest(TString tstrName);

	//!destroys the test case
	~AnythingPerfTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!describe this testcase
	void LookupTest();

	//!describe this testcase
	void DeepCloneTest();
	void PrintOnTest();

	void ExportCsvStatistics();

protected:
	template <typename T> void DoFunctorTest(T value, const char *pName, LoopFunctor pFunc);
	void RunIndexLoopAsCharPtr(long index, const Anything &a, const long iterations);
	void RunIndexLoopAsString(long index, const Anything &a, const long iterations);
	void RunKeyLoop(const char *key, const Anything &a, const long iterations);
	void RunLookupPathLoop(const char *key, const Anything &a, const long iterations);
	void RunROLookupPathLoop(const char *key, const ROAnything &a, const long iterations);
	void RunDeepCloneLoop(const char *pName, const Anything &a, const long iterations);
	void RunPrintOnPrettyLoop(const char *pName, const Anything &a, const long iterations);
};

#endif
