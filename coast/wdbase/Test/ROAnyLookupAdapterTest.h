/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ROAnyLookupAdapterTest_H
#define _ROAnyLookupAdapterTest_H

#include "TestCase.h"

//---- ROAnyLookupAdapterTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class ROAnyLookupAdapterTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	ROAnyLookupAdapterTest(TString tstrName);

	//! destroys the test case
	~ROAnyLookupAdapterTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	void LookupTest();
};

#endif
