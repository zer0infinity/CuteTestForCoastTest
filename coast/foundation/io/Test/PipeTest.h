/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PipeTest_H
#define _PipeTest_H

#include "TestCase.h"

//---- PipeTest ----------------------------------------------------------
//!TestCases description
class PipeTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	PipeTest(TString tstrName);

	//!destroys the test case
	~PipeTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!test the constructor Pipe(fdin,fdout,doclose,timeout)
	void simpleConstructorTest();
	//!test the constructor Pipe(doclose,timeout)
	void defaultConstructorTest();
	//!test the blocking of a pipe itself by writing a lot of stuff.
	void simpleBlockingTest();
};

#endif
