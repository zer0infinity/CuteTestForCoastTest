/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPFlowControllerTest_H
#define _HTTPFlowControllerTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredActionTest.h"
#include "Context.h"

//---- HTTPFlowControllerTest ----------------------------------------------------------
//!TestCases description
class HTTPFlowControllerTest : public ConfiguredActionTest
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	HTTPFlowControllerTest(TString tstrName);

	//!TestCase constructor
	//! \param name name of the test
	//! \param configFileName name of the tests config file
	HTTPFlowControllerTest(TString tstrName, TString configFileName);

	//!destroys the test case
	~HTTPFlowControllerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	TString getConfigFileName();

protected:
	//! Generic Test run - driven by TestCase Name
	virtual void DoTest(Anything testCase, const char *testCaseName);
	Context fCtx;
};

#endif
