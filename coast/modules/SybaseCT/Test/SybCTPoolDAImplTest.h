/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTPoolDAImplTest_H
#define _SybCTPoolDAImplTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"

//---- SybCTPoolDAImplTest ----------------------------------------------------------
//:TestCases description
class SybCTPoolDAImplTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//:TestCase constructor
	//!param: name - name of the test
	SybCTPoolDAImplTest(TString tstrName);

	//:destroys the test case
	~SybCTPoolDAImplTest();

	//--- public api

	//:builds up a suite of testcases for this test
	static Test *suite ();

	//:describe this testcase
	void SimpleDATest();
};

#endif
