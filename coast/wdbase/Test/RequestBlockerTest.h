/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestBlockerTest_H
#define _RequestBlockerTest_H

#include "TestCase.h"

//---- RequestBlockerTest ----------------------------------------------------------
//!test for RequestBlocker, migrated from mtfoundation/Test/ThreadsTest
class RequestBlockerTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	RequestBlockerTest(TString tstrName);

	//!destroys the test case
	~RequestBlockerTest();

	//--- public api
	//!hook for thread into testcase
	virtual void QueryRB(bool compare, bool checkIt, TString tname);
	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!test RB singleton, which is a RWLock implementtion
	virtual void RBTest();
};

#endif
