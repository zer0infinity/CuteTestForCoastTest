/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Base64RendererTest_H
#define _Base64RendererTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- Base64RendererTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class Base64RendererTest : public TestCase
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	Base64RendererTest(TString tstrName);

	//! destroys the test case
	~Base64RendererTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! describe this testcase
	void RenderAllTest();
};

#endif
