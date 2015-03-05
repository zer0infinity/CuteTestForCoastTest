/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StatementDescriptionTest_H
#define _StatementDescriptionTest_H

#include "FoundationTestTypes.h"

class StatementDescriptionTest : public testframework::TestCaseWithConfig
{
public:
	//!constructors
	StatementDescriptionTest(TString tstrName);
	~StatementDescriptionTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!tests pathlist accessor
	void DescriptionElementConstructionTest();
	void DescriptionElementShadowTest();
	void DescriptionSimpleTest();
	void DescriptionIteratorTest();
};

#endif
