/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _URI2FileNameTest_H
#define _URI2FileNameTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- URI2FileNameTest ----------------------------------------------------------
//!TestCases description
class URI2FileNameTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	URI2FileNameTest(TString tstrName);

	//!destroys the test case
	~URI2FileNameTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!describe this testcase
	void Uri2FilenameTest();

protected:

};

#endif
