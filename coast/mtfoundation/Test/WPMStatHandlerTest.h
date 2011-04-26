/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WPMStatHandlerTest_H
#define _WPMStatHandlerTest_H

#include "TestCase.h"

class WPMStatHandler;
class Anything;
class WPMStatHandlerTest: public TestFramework::TestCase {
public:
	WPMStatHandlerTest(TString tname) :
		TestCaseType(tname) {
	}
	//!builds up a suite of testcases for this test
	static Test *suite();

	//!test the state of the statistics after construction
	void ConstructorTest();
	//!test behavior for the statistic events
	void StatEvtTests();
	//!test behavior for the statistic call
	void StatisticTests();

protected:
	//!bottleneck to assert state
	bool AssertState(const WPMStatHandler &wpm, const Anything &state);
};

#endif
