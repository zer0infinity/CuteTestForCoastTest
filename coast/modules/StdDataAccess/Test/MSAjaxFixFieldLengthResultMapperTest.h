/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#ifndef MSAJAXFIXFIELDLENGTHRESULTMAPPERTEST_H_
#define MSAJAXFIXFIELDLENGTHRESULTMAPPERTEST_H_

#include "FoundationTestTypes.h"

class MSAjaxFixFieldLengthResultMapperTest: public TestFramework::TestCaseWithConfig {
public:
	//! ConfiguredTestCase constructor
	//! \param name name of the test
	MSAjaxFixFieldLengthResultMapperTest(TString tstrName) :
		TestCaseType(tstrName) {
	}

	//! builds up a suite of tests
	static Test *suite();

	//! take a simple http response and parse it
	void ConfiguredTests();
};

#endif /* MSAJAXFIXFIELDLENGTHRESULTMAPPERTEST_H_ */
