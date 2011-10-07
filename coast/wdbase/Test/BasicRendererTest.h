/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BasicRendererTest_H
#define _BasicRendererTest_H

#include "TestCase.h"

class BasicRendererTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//@param: name - name of the test
	BasicRendererTest(TString tstrName);

	//!destroys the test case
	~BasicRendererTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!test the rendering of a simple string
	void RenderASimpleString();
	//!render something onto a string
	void RenderOnStringTest();
	//!render something onto a string using a default
	void RenderOnStringDefaultTest();
	//!render a simple list
	void RenderASimpleList();
	//!render a simple config using a mock class TestRenderer
	void RenderWithConfig();
};

#endif
