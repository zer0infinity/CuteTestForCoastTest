/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ActionCoreTest_H
#define _ActionCoreTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- ActionCoreTest ----------------------------------------------------------
//!TestCases description
class ActionCoreTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	ActionCoreTest(TString name);

	//!destroys the test case
	~ActionCoreTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!checks if the transition token is handled according to the unconfigured Action
	void ConsistentTransitionHandling();

	//!The same semantic as in ConsistentTransitionHandling but using the new API function
	void EmptyConfigurationTransitionHandling();

	//!checks if the transition token is handled according to the configured Action
	void ConfiguredTransitionHandling();

	//!runs a sequence of actions
	void ActionSequence();

	//!runs a sequence of actions until the first one fail
	void AbortedActionSequence();

protected:

};

#endif
