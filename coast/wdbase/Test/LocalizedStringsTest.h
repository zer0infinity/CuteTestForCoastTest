/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LocalizedStringsTest_H
#define _LocalizedStringsTest_H

#include "WDBaseTestPolicies.h"

class LocalizedStringsTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//!ConfiguredTestCase constructor
	//! \param name name of the test
	LocalizedStringsTest(TString tstrName);

	//!destroys the test case
	~LocalizedStringsTest();

	//--- public api

	//!builds up a suite of tests
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	TString getConfigFileName();

	//!describe this Test
	void test();
};

#endif
