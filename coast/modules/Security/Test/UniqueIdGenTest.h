/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _UniqueIdGenTest_H
#define _UniqueIdGenTest_H

#include "TestCase.h"

class String;

class UniqueIdGenTest : public TestFramework::TestCase
{
public:
	/*! \param tstrName name of the test */
	UniqueIdGenTest(TString tstrName) :
			TestCaseType(tstrName) {
	}

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! describe this testcase
	void GetUniqueIdTest();

protected:
	void DoGetUniqueIdTest(long iterations, const String &additionalToken);
};

#endif
