/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPDAICachePolicyTest_H
#define _LDAPDAICachePolicyTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "Anything.h"

//---- LDAPDAICachePolicyTest ----------------------------------------------------------
//:TestCases description
class LDAPDAICachePolicyTest : public TestCase
{
public:
	//--- constructors

	//:TestCase constructor
	//!param: name - name of the test
	LDAPDAICachePolicyTest(TString name);

	//!destroys the test case
	~LDAPDAICachePolicyTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();
	void setUp (const String &configName);

	//!deletes the environment for this test
	void tearDown ();

	//!Do some reinits to check behaviour on reset.
	void ReInitTest();

	//!Do same calls in a row, should use cache
	void CallsInARow();

	//!LDAP query doesn't deliver any data, Module init should fail
	void NoDataReadTest();

	//!describe this testcase
	void testCase();
protected:
	Anything fGlobalConfig;
};

#endif
