/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WDModuleTest_H
#define _WDModuleTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------
class Registry;

//---- WDModuleTest ----------------------------------------------------------
//!TestCases for Coast state transitions
class WDModuleTest : public TestCase
{
public:
	//!TestCases
	//! \param name name of the test
	WDModuleTest(TString name);

	//!destroys the test case
	~WDModuleTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	void InstallTest ();
	void Install2Test ();
	void ResetTest ();
	void TerminateTest ();
	void ResetWithDiffConfigsTest ();

protected:
	Registry *fOrigWDModuleRegistry;
	//--- member variables declaration

private:
	// use careful, you inhibit subclass use
	//--- private class api
	//--- private member variables
};

#endif
