/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTnewDAImplTest_H
#define _SybCTnewDAImplTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- SybCTnewDAImplTest ----------------------------------------------------------
//:TestCases description
class SybCTnewDAImplTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//:TestCase constructor
	//!param: name - name of the test
	SybCTnewDAImplTest(TString tstrName);

	//:destroys the test case
	~SybCTnewDAImplTest();

	//--- public api

	//:builds up a suite of testcases for this test
	static Test *suite ();

	//:sets the environment for this test
	void setUp ();

	//:deletes the environment for this test
	void tearDown ();

	//:describe this testcase
	void UninitializedExecTest();
	void InitTest();
	void DoGetConnectionTest();
	void DoPutbackConnectionTest();
	void SimpleDATest();

	bool fbWasInitialized;
};

#endif
