/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServerTest_H
#define _ServerTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

//---- forward declaration -----------------------------------------------
class Registry;

//---- ServerTest ----------------------------------------------------------
//!TestCases for the Registry class
class ServerTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//!TestCases for the Registry classes
	//! \param name name of the test
	ServerTest(TString name);

	//!destroys the test case
	~ServerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//! test initialization - run - termination sequences
	void InitRunTerminateTest();

	//! test initialization - run - reset - run - termination sequences
	void InitRunResetRunTerminateTest();

	//! run a test sequence
	void RunTestSequence();

protected:

private:

};

#endif
