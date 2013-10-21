/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPDAICachePolicyTest_H
#define _LDAPDAICachePolicyTest_H

#include "WDBaseTestPolicies.h"

//---- LDAPDAICachePolicyTest ----------------------------------------------------------
//:TestCases description
class LDAPDAICachePolicyTest : public testframework::TestCaseWithCaseConfigDllAndModuleLoading
{
public:
	//--- constructors

	//:TestCase constructor
	//!param: name - name of the test
	LDAPDAICachePolicyTest(TString tstrName);

	//!destroys the test case
	~LDAPDAICachePolicyTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!Do some reinits to check behaviour on reset.
	void ReInitTest();

	//!Do same calls in a row, should use cache
	void CallsInARow();

	//!LDAP query doesn't deliver any data, Module init should fail
	void NoDataReadTest();
};

#endif
