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
#include "ConfiguredTestCase.h"

class OTPList;
class TokenDataAccessController;

//---- OTPListTest ----------------------------------------------------------
//! <B>Tests OTPList implementations.</B>
class OTPListTest : public ConfiguredTestCase
{
public:
	//--- constructors

	/*! \param name name of the test and its configuration file */
	OTPListTest(TString tstrName);

	//! destroys the test case
	~OTPListTest();

	//--- public api

	//! builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! tests the implementation of a MockOTPList with a config
	void testMockOTPList();

	//! tests the implementation of a MockOTPList without a config
	void testAlwaysTrueMockOTPList();

private:
	//! run standard and extra tests for a given OtpListImpl
	void RunOtpTests(String name, OTPList *impl, TokenDataAccessController *tdac);
};

#endif
