/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnySorterTest_H
#define _AnySorterTest_H

#include "TestCase.h"

//---- forward declaration -----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "Anything.h"

//---- AnySorterTest -----------------------------------------------------------
//!testcases for AnySorterTest
class AnySorterTest : public TestCase
{
	Anything fConfig;
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	AnySorterTest(TString name);

	//! destroys the test case
	~AnySorterTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! describe this testcase
	void testCase();

	void testAnySorter();
};
#endif
