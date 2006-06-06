/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FlowControllerTest_H
#define _FlowControllerTest_H

//---- baseclass include -------------------------------------------------
#include "StressAppTest.h"

//---- FlowControllerTest ----------------------------------------------------------
//!TestCases description
class FlowControllerTest : public StressAppTest
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	FlowControllerTest(TString tstrName);

	//!destroys the test case
	~FlowControllerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!Test with Full configuration
	void FullConfigurationTest();

	//!Test with configuration without NumberOfRuns ( default of one is taken )
	void WithoutNumberOfRunsConfigurationTest();

	//!Test with configuration with NumberOfRuns=0 ( run is not executed )
	void NumberOfRuns0ConfigurationTest();

	//!Test with configuration without PreRun (just Run executed )
	void WithoutPreRunConfigurationTest();

	//!Test with configuration without Run ( just PreRun executed )
	void WithoutRunConfigurationTest();

	//!Test with configuration with only PreRun
	void PreRunOnlyConfigurationTest();

	//!Test with configuration with only Run
	void RunOnlyConfigurationTest();

	//!Test with configuration with only Run and only one step
	void RunOnlyOneStepConfigurationTest();

	//!Test with configuration that does not contain any of the necessary slots
	void InvalidConfigurationTest();

protected:
	//! Generic Test run - driven by TestCase Name
	//!Checks if the tmpStore is prepared according the FlowController's configuration
	void DoTest();
	Anything fConfig;
};

#endif
