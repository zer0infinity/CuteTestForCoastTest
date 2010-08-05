/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _OTPListTest_H
#define _OTPListTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"

class OTPList;
class TokenDataAccessController;

//---- OTPListTest ----------------------------------------------------------
//! Tests OTPList implementations.
class OTPListTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	/*! \param name name of the test and its configuration file */
	OTPListTest(TString tstrName);

	//! destroys the test case
	~OTPListTest();

	//--- public api

	//! builds up a suite of tests
	static Test *suite ();

	//! tests the implementation of a MockOTPList with a config
	void MockOTPListTest();

	//! tests the implementation of a MockOTPList without a config
	void AlwaysTrueMockOTPListTest();

	TString getConfigFileName();

private:
	//! run standard and extra tests for a given OtpListImpl
	void RunOtpTests(String name, OTPList *impl, TokenDataAccessController *tdac);
};

#endif
