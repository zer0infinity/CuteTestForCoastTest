/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SocketTest_H
#define _SocketTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "Anything.h"
//---- SocketTest ----------------------------------------------------------
//!TestCases for the Socket class
class SocketTest : public TestCase
{
public:
	//--- constructors

	//!TestCases for the Socket classes
	//! \param name name of the test
	SocketTest(TString name);

	//!destroys the test case
	~SocketTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!tests the Socket class with a fd from Connector
	void simpleConstructorTest();

	//!tests the Socket class with a invalid fd
	void faultyConstructorTest();

	//!tests the Socket class with a fd from Connector with a pool allocator
	void allocatorConstructorTest();

	//!tests the Socket class with a http server interaction as client
	void httpClientTest();

	//!tests the Socket class with a nonexistent server interaction as client
	void faultyClientTest();

	//!tests the conversion of ip-addr to network address
	void MakeInetAddrTest();

	//!tests the extension of the clientinfo Anything for use by keepalive connections
	void AppendToClientInfoTest();

	void SetToNonBlockingTest();

	//--- subclass api
	static bool IsNonBlocking(int fd);

private:
	Anything	fConfig;
};

#endif
