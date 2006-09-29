/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTnewDATest_H
#define _SybCTnewDATest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"
#include "cstypes.h"

//---- SybCTnewDATest ----------------------------------------------------------
//:TestCases description
class SybCTnewDATest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//:TestCase constructor
	//!param: name - name of the test
	SybCTnewDATest(TString tstrName);

	//:destroys the test case
	~SybCTnewDATest();

	//--- public api

	//:builds up a suite of testcases for this test
	static Test *suite ();

	//:describe this testcase
	void InitOpenSetConPropTest();
	void SimpleQueryTest();
	void LimitedMemoryTest();
	void LoginTimeoutTest();
	void ResultTimeoutTest();

private:
	void IntLoginTimeoutTest(CS_CONTEXT *context, long lMaxNumber, long lCurrent);
};

#endif
