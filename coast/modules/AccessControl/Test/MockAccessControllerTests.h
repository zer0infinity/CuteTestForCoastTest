/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MockAccessControllerTests_H
#define _MockAccessControllerTests_H

//---- baseclass include -------------------------------------------------
#include "FileAccessControllerTests.h"

//---- MockAccessControllerTests ------------------------------------------
//! Tests the MockAccessControllers (MockUDAC, MockTDAC, MockEDAC)
/*!
Each of the tested AccessControllers has an own config file.
*/
class MockAccessControllerTests : public FileAccessControllerTests
{
public:
	//--- constructors
	MockAccessControllerTests(TString tstrName)
		: FileAccessControllerTests(tstrName)
	{}
	~MockAccessControllerTests() {};

	//--- public api

	//! builds up a suite of tests
	static Test *suite ();

	TString getConfigFileName() {
		return "MockAccessControllerTestsConfig";
	}

	void MockUDACTest();
	void MockTDACTest();
	void MockEDACTest();
};

#endif
