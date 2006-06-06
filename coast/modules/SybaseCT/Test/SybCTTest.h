/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTTest_H
#define _SybCTTest_H

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- SybCTTest ----------------------------------------------------------
//:TestCases description
class SybCTTest : public TestFramework::TestCaseWithConfig
{
public:
	//--- constructors

	//:TestCase constructor
	//!param: name - name of the test
	SybCTTest(TString tstrName);

	//:destroys the test case
	~SybCTTest();

	//--- public api

	//:builds up a suite of testcases for this test
	static Test *suite ();

	//:describe this testcase
	void SybCTTestTest();
	void LimitedMemoryTest();
};

#endif
