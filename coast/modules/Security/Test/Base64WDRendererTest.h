/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Base64WDRendererTest_H
#define _Base64WDRendererTest_H

#include "TestCase.h"

//---- Base64WDRendererTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/

class Base64WDRendererTest : public testframework::TestCase
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	Base64WDRendererTest(TString name);

	//! destroys the test case
	~Base64WDRendererTest();

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
