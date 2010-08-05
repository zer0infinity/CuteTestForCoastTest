/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPConnectionTest_H
#define _LDAPConnectionTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"

//---- LDAPConnectionTest ----------------------------------------------------------
//! Tests LDAPConnecton and LDAPErrorHandler
class LDAPConnectionTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	/*! \param name name of the test and its configuration file */
	LDAPConnectionTest(TString tstrName);

	//! destroys the test case
	~LDAPConnectionTest();

	//--- public api

	//! builds up a suite of tests
	static Test *suite ();

	TString getConfigFileName();

	//! describe this Test
	void ConnectionTest();
};

#endif
