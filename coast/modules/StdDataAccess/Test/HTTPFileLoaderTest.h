/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPFileLoaderTest_H
#define _HTTPFileLoaderTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- HTTPFileLoaderTest ----------------------------------------------------------
//!TestCases description
class HTTPFileLoaderTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	HTTPFileLoaderTest(TString name);

	//!destroys the test case
	~HTTPFileLoaderTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!test the reply header expansion
	void ReplyHeaderTest();

	//!test the reply header expansion
	void ExecTest();
};

#endif
