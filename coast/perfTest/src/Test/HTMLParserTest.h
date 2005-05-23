/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLParserTest_H
#define _HTMLParserTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- HTMLParserTest ----------------------------------------------------------
//!TestCases description
class HTMLParserTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	HTMLParserTest(TString tstrName);

	//!destroys the test case
	~HTMLParserTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!describe this testcase
	void ParseFileTest();

protected:

};

#endif
