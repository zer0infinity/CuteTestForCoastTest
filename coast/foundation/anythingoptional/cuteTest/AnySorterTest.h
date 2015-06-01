/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnySorterTest_H
#define _AnySorterTest_H

#include "cute.h"
#include "Tracer.h"
#include "AssertionAnything.h"
#include "SystemFile.h"

class AnySorterTest : public Assertion {
public:
	//! TestCase constructor
	//! \param name name of the test
	AnySorterTest();

	//! destroys the test case
	~AnySorterTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static void runAllTests(cute::suite &s);
	ROAnything GetTestCaseConfig(String strClassName = "", String strTestName = "");
	void SorterTest();
};
#endif
