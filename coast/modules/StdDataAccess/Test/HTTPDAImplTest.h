/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPDAImplTest_H
#define _HTTPDAImplTest_H

#include "WDBaseTestPolicies.h"

//---- HTTPDAImplTest ----------------------------------------------------------
//!TestClass for for HTMLParser functionality

class HTTPDAImplTest : public testframework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:

	//--- constructors
	HTTPDAImplTest(TString tstrName);
	~HTTPDAImplTest() {};

	//--- public api
	//!generate HTMLParser test suite
	//! this method generates the tests for the HTMLParser classed
	//! \return a new test is created by this method
	static Test *suite ();

	TString getConfigFileName();

protected:
	//--- tests for public api of class connector

	//!tests htmlparser with a file from config
	void useSSLTest();

	//!tests htmlparser with a file from config
	void ErrorHandlingTest();

	void SSLTests();

	void SSLNirvanaConnectTests();
};

#endif
