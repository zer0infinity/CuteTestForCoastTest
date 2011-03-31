/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RemoteStresserTest_H
#define _RemoteStresserTest_H

#include "StressAppTest.h"

class Server;
class ServerThread;

//---- RemoteStresserTest ----------------------------------------------------------
//!TestCases for the RemoteStresser class
//! These tests have to start a stress server first
class RemoteStresserTest : public StressAppTest
{
public:
	//--- constructors

	//!TestCases for the StressApp classes
	//! \param name name of the test
	RemoteStresserTest(TString tstrName);

	//!destroys the test case
	~RemoteStresserTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	void tearDown();

	// the testcases
	void TestRemoteStresser();
	void TestMultiRemoteStresser();

protected:
	ServerThread *fServerRunner;
	Server *fStressServer;
};

#endif
