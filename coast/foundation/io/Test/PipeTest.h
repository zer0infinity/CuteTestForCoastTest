/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PipeTest_H
#define _PipeTest_H

#include "TestCase.h"

class PipeTest: public TestFramework::TestCase {
public:
	PipeTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void simpleConstructorTest();
	void defaultConstructorTest();
	void simpleBlockingTest();
};

#endif
