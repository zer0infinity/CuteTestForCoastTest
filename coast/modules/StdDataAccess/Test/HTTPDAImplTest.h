/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPDAImplTest_H
#define _HTTPDAImplTest_H

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- HTTPDAImplTest ----------------------------------------------------------
//!TestClass for for HTMLParser functionality

class HTTPDAImplTest : public TestFramework::TestCaseWithConfig
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

	//!generate set up for connector test does nothing so far
	void setUp ();

	//!delete set up for connector test does nothing so far
	void tearDown ();

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
