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
#include "FoundationTestTypes.h"

//---- SSLModuleTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class SSLModuleTest : public TestFramework::TestCaseWithConfig
{
public:
	//--- constructors

	/*! \param name name of the test */
	SSLModuleTest(TString tstrName);

	//! destroys the test case
	~SSLModuleTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	TString getConfigFileName();

	//! describe this testcase
	void LoadCertAndKeyTest();
};

#endif

