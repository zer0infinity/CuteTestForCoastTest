/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServiceDispatcherTest_H
#define _ServiceDispatcherTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"

//---- ServiceDispatcherTest ----------------------------------------------------------
//!Tests dispatching of standard handler
class ServiceDispatcherTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	ServiceDispatcherTest(TString tstrName);

	//!destroys the test case
	~ServiceDispatcherTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!tests the find protocol of the service dispatcher class
	void FindTests();

	//!tests the ServiceDispatcherModule
	void ServiceDispatcherModuleTest();
};

#endif
