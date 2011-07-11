/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AssertionTest_H
#define _AssertionTest_H

#include "TestCase.h"//lint !e537

//---- AssertionTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class AssertionTest : public TestFramework::TestCase
{
public:
	//--- constructors

	/*! \param tstrName name of the test */
	AssertionTest(TString tstrName);

	//! destroys the test case
	~AssertionTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! describe this testcase
	void MyCompareTest() {
		assertCompare(13, less, 14);
		assertCompare(13LL, less, 14LL);
		assertCompare(13UL, less, 14UL);
		assertCompare(16, less, 14);
		assertCompare(15LL, less, 14LL);
		assertCompare(17UL, less, 14UL);
		assertCompare(17ULL, less, 14ULL);
		assertComparem(15, less, 14, "just to show that it is not less");
		assertComparem( 14, less, 14, "it is equal but not less");
		assertCompare( 14, less_equal, 14);
		assertComparem( 13, greater, 14, "not greater test to fail");
		assertCompare( 14, greater_equal, 14);
		assertCompare( 14, equal_to, 14);
		assertCompare( 15, not_equal_to, 14);
	}
};

#endif
