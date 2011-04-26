/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPFlowControllerPrepareTest_H
#define _HTTPFlowControllerPrepareTest_H

#include "HTTPFlowControllerTest.h"
class HTTPFlowControllerPrepareTest: public HTTPFlowControllerTest {
public:
	//!TestCase constructor
	//! \param name name of the test
	HTTPFlowControllerPrepareTest(TString tstrName) :
		HTTPFlowControllerTest(tstrName) {
	}
	//!TestCase constructor
	//! \param name name of the test
	//! \param configFileName name of the tests config file
	HTTPFlowControllerPrepareTest(TString tstrName, TString configFileName);

	void DoCommandTest();
	void ProcessSetCookieTest();
	static Test *suite();
	TString getConfigFileName();
};

#endif
