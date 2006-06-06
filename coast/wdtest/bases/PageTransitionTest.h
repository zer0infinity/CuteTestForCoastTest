/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PageTransitionTest_H
#define _PageTransitionTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredActionTest.h"

//---- PageTransitionTest ----------------------------------------------------------
//! Test cases that perform the whole page event handling and transition
class PageTransitionTest : public ConfiguredActionTest
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	PageTransitionTest(TString tstrName);

	//!destroys the test case
	virtual ~PageTransitionTest();

	//!sets up the test case
	virtual void setUp();

	TString getConfigFileName();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//! Runs the Testcases from PageTransitionTestConfig.any
	void RunTestCases();

protected:
	//! performs the test with configuration config
	//! \param config in - test case configuration (env, query, Results)
	//! \param testCaseName in - name of the test for output
	virtual void DoTest(Anything config, const char *testCaseName);

	//! Prepares the config for the TestCase
	//! \param originalConfig the original Configuration
	//! \return if originalConfig contains a slot UseConfig the config of the TestCase specified there is returned , else originalConfig
	//! A Slot /Replace defines additions and replacement to the originalConfig.
	Anything PrepareConfig(Anything originalConfig);

	class Server *fServer;
};

#endif
