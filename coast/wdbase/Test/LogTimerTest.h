/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LogTimerTest_H
#define _LogTimerTest_H

#include "TestCase.h"

class LogTimerTest: public testframework::TestCase {
public:
	LogTimerTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void MethodTimerTest();
};

#endif
