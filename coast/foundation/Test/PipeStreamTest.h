/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PipeStreamTest_H
#define _PipeStreamTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- PipeStreamTest ----------------------------------------------------------
//!TestCases description
class PipeStreamTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	PipeStreamTest(TString name);

	//!destroys the test case
	~PipeStreamTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!simple write and read
	void SimpleWriteandRead();
	//!simple write and read
	void MoreWriteandRead();
	//!simple write and read in a loop so it timeouts
	void LoopWriteandRead();
	//!have two processes communicating
	void PipeAndFork();

protected:

};

#endif
