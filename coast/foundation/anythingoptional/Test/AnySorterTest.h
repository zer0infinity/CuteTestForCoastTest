/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnySorterTest_H
#define _AnySorterTest_H

#include "FoundationTestTypes.h"

class AnySorterTest : public testframework::TestCaseWithConfig
{
public:
	//! TestCase constructor
	//! \param name name of the test
	AnySorterTest(TString tstrName);

	//! destroys the test case
	~AnySorterTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	void SorterTest();
};
#endif
