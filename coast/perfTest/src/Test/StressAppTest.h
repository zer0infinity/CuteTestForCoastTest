/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StressAppTest_H
#define _StressAppTest_H

#include "TestCase.h"

class ROAnything;

class StressAppTest: public TestFramework::TestCase {
public:
	StressAppTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void AppRunTest();
	void DataAccessStresserTest();
	void ThreadedStresserRunnerTest();
	void StressProcessorTest();
	void FlowControlDAStresserTest();

protected:
	//! utility method to check consistency of result
	void CheckSum(ROAnything result);
};

#endif
