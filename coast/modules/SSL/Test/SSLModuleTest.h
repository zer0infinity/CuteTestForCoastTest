/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SSLModuleTest_H
#define _SSLModuleTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

//---- SSLModuleTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class SSLModuleTest : public ConfiguredTestCase
{
public:
	//--- constructors

	/*! \param name name of the test */
	SSLModuleTest(TString name);

	//! destroys the test case
	~SSLModuleTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! describe this testcase
	void testCase();
	void testLoadCertAndKey();
};

#endif

