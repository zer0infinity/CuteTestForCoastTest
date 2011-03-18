/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MIMEHeaderTest_H
#define _MIMEHeaderTest_H

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- MIMEHeaderTest ----------------------------------------------------------
class MIMEHeaderTest: public TestFramework::TestCaseWithConfig {
public:
	//!TestCase constructor
	//! \param name name of the test
	MIMEHeaderTest(TString tstrName) :
		TestCaseType(tstrName) {
	}

	//!builds up a suite of testcases for this test
	static Test *suite();

	void SimpleHeaderTest();
	void ConfiguredTests();
	void spiritTests();
	void testrfcheaders();
};

#endif
