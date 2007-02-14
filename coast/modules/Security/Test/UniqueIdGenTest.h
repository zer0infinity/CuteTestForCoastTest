/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _UniqueIdGenTest_H
#define _UniqueIdGenTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

class String;

//---- UniqueIdGenTest ----------------------------------------------------------
class UniqueIdGenTest : public TestFramework::TestCase
{
public:
	//--- constructors

	/*! \param tstrName name of the test */
	UniqueIdGenTest(TString tstrName);

	//! destroys the test case
	~UniqueIdGenTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! describe this testcase
	void GetUniqueIdTest();

protected:
	void DoGetUniqueIdTest(long iterations, const String &additionalToken);
};

#endif
