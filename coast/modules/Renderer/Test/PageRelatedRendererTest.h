/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PageRelatedRendererTest_H
#define _PageRelatedRendererTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------

//---- PageRelatedRendererTest ----------------------------------------------------------
//!Test driver for tracing facility
class PageRelatedRendererTest : public TestCase
{
public:
	//!constructors
	PageRelatedRendererTest(TString name);
	~PageRelatedRendererTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

protected:
	//--- subclass api

	void trivialTest();

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

#endif
