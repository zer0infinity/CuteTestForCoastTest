/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DiffTimerTest_H
#define _DiffTimerTest_H

#include "TestCase.h"

class DiffTimerTest : public testframework::TestCase
{
public:
	//--- constructors
	DiffTimerTest(TString tstrName);
	~DiffTimerTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!tests assumption about the constructor
	void ConstructorTest();
	//!tests scaling of time measurements beware of overflows
	void ScaleTest();
	//!a simple time test
	void DiffTest();
	//!a simulated time test
	void SimulatedDiffTest();
};

#endif
