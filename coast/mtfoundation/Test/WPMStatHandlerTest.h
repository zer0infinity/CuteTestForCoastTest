/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WPMStatHandlerTest_H
#define _WPMStatHandlerTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

class WPMStatHandler;

//---- WPMStatHandlerTest ----------------------------------------------------------
//!TestCases description
class WPMStatHandlerTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	WPMStatHandlerTest(TString tstrName);

	//!destroys the test case
	~WPMStatHandlerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

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
