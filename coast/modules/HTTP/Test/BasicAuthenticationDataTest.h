/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BasicAuthenticationDataTest_H
#define _BasicAuthenticationDataTest_H

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- BasicAuthenticationDataTest ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class BasicAuthenticationDataTest : public TestFramework::TestCaseWithConfig
{
public:
	//--- constructors

	//! ConfiguredTestCase constructor
	//! \param name name of the test
	BasicAuthenticationDataTest(TString tstrName);

	//! destroys the test case
	~BasicAuthenticationDataTest();

	//--- public api

	//! builds up a suite of tests
	static Test *suite ();

	TString getConfigFileName();

	//! describe this Test
	void AuthenticationTest();
};

#endif
