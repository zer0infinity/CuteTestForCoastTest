/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _InterruptHandlerTest_H
#define _InterruptHandlerTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

//---- InterruptHandlerTest ----------------------------------------------------------
//!TestCases description
class InterruptHandlerTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	InterruptHandlerTest(TString tstrName);

	//!destroys the test case
	~InterruptHandlerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!describe this testcase
	void PidFileHandlingTest();

protected:

};

#endif
