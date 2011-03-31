/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LdapCachePolicyTest_H
#define _LdapCachePolicyTest_H

#include "WDBaseTestPolicies.h"

//---- LdapCachePolicyTest ----------------------------------------------------------
//:TestCases description
class LdapCachePolicyTest: public TestFramework::TestCaseWithCaseConfigDllAndModuleLoading {
public:
	LdapCachePolicyTest(TString tstrName) :
		TestCaseType(tstrName) {
	}

	//!builds up a suite of testcases for this test
	static Test *suite();

	//!Do some reinits to check behaviour on reset.
	void ReInitTest();

	//!Do same calls in a row, should use cache
	void CallsInARow();

	//!LDAP query doesn't deliver any data, Module init should fail
	void NoDataReadTest();
};

#endif
