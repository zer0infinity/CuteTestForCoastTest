/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SysLogTest_H
#define _SysLogTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- SysLogTest ----------------------------------------------------------
//!testcases for the SysLog class
class SysLogTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	SysLogTest(TString tstrName);

	//!destroys the test case
	~SysLogTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!test the environment
	void TestFlags ();

protected:

};

#endif
