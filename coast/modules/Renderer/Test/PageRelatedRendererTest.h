/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PageRelatedRendererTest_H
#define _PageRelatedRendererTest_H

#include "TestCase.h"

//---- PageRelatedRendererTest ----------------------------------------------------------
//!Test driver for tracing facility
class PageRelatedRendererTest : public TestFramework::TestCase
{
public:
	//!constructors
	PageRelatedRendererTest(TString tstrName);
	~PageRelatedRendererTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

protected:
	void trivialTest();
};

#endif
