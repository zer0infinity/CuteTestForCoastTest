/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RendererDispatcherTest_H
#define _RendererDispatcherTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- RendererDispatcherTest ----------------------------------------------------------
//!TestCases description
class RendererDispatcherTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	RendererDispatcherTest(TString name);

	//!destroys the test case
	~RendererDispatcherTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!test the find of rendered service names
	void FindServiceNameTest();

protected:

};

#endif
