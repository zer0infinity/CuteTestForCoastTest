/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringPerfTest_H
#define _StringPerfTest_H

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- StringPerfTest ----------------------------------------------------------
//!TestCases description
class StringPerfTest : public TestFramework::TestCaseWithStatistics
{
public:
	//!TestCase constructor
	//! \param name name of the test
	StringPerfTest(TString tstrName);

	//!destroys the test case
	~StringPerfTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!describe this testcase
	void referenceTest();

protected:
	void RunLoop(const char *str, const long iterations);
	void RunPreallocLoop(const char *str, const long iterations);
	void RunPoolAllocLoop(const char *str, const long iterations);
};

#endif
