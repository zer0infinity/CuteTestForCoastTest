/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPFlowControllerPrepareTest_H
#define _HTTPFlowControllerPrepareTest_H

//---- baseclass include -------------------------------------------------
#include "HTTPFlowControllerTest.h"
//---- HTTPFlowControllerPrepareTest ----------------------------------------------------------
//!TestCases description
class HTTPFlowControllerPrepareTest : public HTTPFlowControllerTest
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	HTTPFlowControllerPrepareTest(TString tstrName);

	//!TestCase constructor
	//! \param name name of the test
	//! \param configFileName name of the tests config file
	HTTPFlowControllerPrepareTest(TString tstrName, TString configFileName);

	//!destroys the test case
	~HTTPFlowControllerPrepareTest();

	//--- public api
	void DoCommandTest();

	//--- Test cookie handling
	void ProcessSetCookieTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();
};

#endif
