/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PageNameRendererTest_H
#define _PageNameRendererTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- PageNameRendererTest ----------------------------------------------------------
//!TestCases description
class PageNameRendererTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	PageNameRendererTest(TString name);

	//!destroys the test case
	~PageNameRendererTest();
	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!describe this testcase
	void RenderTest();
};

#endif
