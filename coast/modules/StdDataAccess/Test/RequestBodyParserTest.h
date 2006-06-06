/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestBodyParserTest_H
#define _RequestBodyParserTest_H

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- RequestBodyParserTest ----------------------------------------------------------
//!TestCases description
class RequestBodyParserTest : public TestFramework::TestCaseWithConfig
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	RequestBodyParserTest(TString tstrName);

	//!destroys the test case
	~RequestBodyParserTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	TString getConfigFileName();

	//!describe this testcase
	void ReadToBoundaryTest();
	//!describe this testcase
	void ParseMultiPartTest();
	//!describe this testcase
	void ReadMultiPartPost();
	void ReadToBoundaryTestWithStreamFailure();
};
#endif
