/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MimeHeaderResultMapperTest_H
#define _MimeHeaderResultMapperTest_H

#include "FoundationTestTypes.h"

//---- MimeHeaderResultMapperTest ----------------------------------------------------------
class MimeHeaderResultMapperTest: public TestFramework::TestCaseWithConfig {
public:
	//! ConfiguredTestCase constructor
	//! \param name name of the test
	MimeHeaderResultMapperTest(TString tstrName) :
		TestCaseType(tstrName) {
	}

	//! builds up a suite of tests
	static Test *suite();

	//! take a simple http response and parse it
	void ConfiguredTests();
};

#endif
