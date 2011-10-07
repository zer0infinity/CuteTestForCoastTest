/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConnectorTest_H
#define _ConnectorTest_H

#include "FoundationTestTypes.h"//lint !e537

class ConnectorTest : public TestFramework::TestCaseWithConfig
{
public:
	//--- constructors
	ConnectorTest(TString tstrName);
	~ConnectorTest();

	//--- public api
	//!generate Connector test suite
	//! this method generates the tests for the Connector classed
	//! \return a new test is created by this method
	static Test *suite ();

	//--- tests for public api of class connector
	//!tests connector constructor with localhost and port 80
	void simpleConstructorTest();

	//!tests connector constructor with localhost and port 80
	void bindingConstructorTest();

	//!tests connector constructor with localhost and port 80
	void timeOutTest();

	//!tests connector constructor with faulty parameters
	void faultyConstructorTest();

	//!tests method MakeSocket
	void makeSocketTest();

	//!tests method MakeSocket with boolean flag == false
	void makeSocketWithReuseTest();

	//!tests method UseSocket
	void useSocketTest();

	//!tests method GetStream
	void getStreamTest();

	//!tests allocator
	void allocatorConstructorTest();

private:
	// use careful, you inhibit subclass use
	//--- private class api
	void ConnectAndAssert(const char *host, long port, long timeout, bool threadLocal, bool shouldFail);
};

#endif
