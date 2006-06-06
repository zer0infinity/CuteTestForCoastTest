/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ResolverTest_H
#define _ResolverTest_H

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- forward declaration -----------------------------------------------

//---- ResolverTest ----------------------------------------------------------
//!TestCases for the Resolver class
class ResolverTest : public TestFramework::TestCaseWithConfig
{
public:
	//!TestCases for the Resolver class
	//! \param name name of the test
	ResolverTest(TString tstrName);

	//!destroys the test case
	~ResolverTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!tests the Socket class with a fd from Connector
	void simpleDNS2IPTest();

	//!tests the Socket class with a invalid fd
	void simpleIP2DNSTest();
};

#endif
