/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SSLConnectorTest_H
#define _SSLConnectorTest_H

#include "FoundationTestTypes.h"
#include "Anything.h"

class SSLConnector;

//---- SSLConnectorTest ----------------------------------------------------------
//!TestClass for for SSLConnector functionality
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class SSLConnectorTest : public TestFramework::TestCaseWithConfig
{
public:

	//--- constructors
	SSLConnectorTest(TString tstrName);
	~SSLConnectorTest();

	//--- public api
	//!generate SSLConnector test suite
	//! this method generates the tests for the SSLConnector classed
	//! \return a new test is created by this method
	static Test *suite ();

	//!generate set up for SSLConnector test does nothing so far
	void setUp ();

	//!delete set up for SSLConnector test does nothing so far
	void tearDown ();

	//--- tests for public api of class SSLConnector

	//!tests SSLConnector constructor with localhost and port 443
	void simpleConstructorTest();

	//!tests SSLConnector constructor with localhost and port 443 and allocator with pool storage
	void allocatorConstructorTest();

	//!tests SSLConnector constructor with localhost and port 443
	void timeOutTest();

	//!tests SSLConnector constructor with faulty parameters
	void faultyConstructorTest();

	//!tests method GetStream
	void getStreamTest();

	//!tests method GetSessionID
	void getSessionIDTest();

private:
	// use careful, you inhibit subclass use
	//--- private class api
	void ConnectAndAssert(const char *host, long port, long timeout, bool shouldFail);

};

#endif
