/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConfiguredLookupAdapterTest_H
#define _ConfiguredLookupAdapterTest_H

#include "FoundationTestTypes.h"

//! LookupAdapter providing a configured default
/*!
The first argument is the "Config" to look up. If the lookup fails,
the second argument is looked up. Unlike the LookupInterface's default
values you can define your own default configuration which is looked up.
See testcases for samples.
*/
class ConfiguredLookupAdapterTest : public TestFramework::TestCaseWithConfig
{
public:
	//--- constructors

	/*! \param name name of the test and its configuration file */
	ConfiguredLookupAdapterTest(TString tstrName);

	//! destroys the test case
	~ConfiguredLookupAdapterTest();

	//--- public api

	//! builds up a suite of tests
	static Test *suite ();

	TString getConfigFileName();

	//! Test lookups for longs and Strings
	void LookupTest();
};

#endif
