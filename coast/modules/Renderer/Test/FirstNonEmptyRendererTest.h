/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FirstNonEmptyRendererTest_H
#define _FirstNonEmptyRendererTest_H

//---- baseclass include -------------------------------------------------
#include "RendererTest.h"

//---- FirstNonEmptyRendererTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class FirstNonEmptyRendererTest : public RendererTest
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	FirstNonEmptyRendererTest(TString tstrName);

	//! destroys the test case
	~FirstNonEmptyRendererTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! render an empty list
	void EmptyList();
	//! render an one element list
	void OneElement();
	//! render an multiple element list, first non-empty is done
	void MultipleElements();
};

#endif
