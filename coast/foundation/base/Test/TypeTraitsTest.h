/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TypeTraitsTest_H
#define _TypeTraitsTest_H

#include "TestCase.h"//lint !e537

//---- TypeTraitsTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class TypeTraitsTest : public TestFramework::TestCase
{
public:
	//--- constructors

	/*! \param tstrName name of the test */
	TypeTraitsTest(TString tstrName);

	//! destroys the test case
	~TypeTraitsTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! describe this testcase
	void TraitsTest();
	void ClassTraitsTest();
};

#endif
