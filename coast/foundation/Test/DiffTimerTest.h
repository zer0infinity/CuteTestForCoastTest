/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DiffTimerTest_H
#define _DiffTimerTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------

//---- DiffTimerTest ----------------------------------------------------------
//!testcases for DiffTimer class
class DiffTimerTest : public TestCase
{
public:
	//--- constructors
	DiffTimerTest(TString tstrName);
	~DiffTimerTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!tests assumption about the constructor
	void ConstructorTest();
	//!tests scaling of time measurements beware of overflows
	void ScaleTest();
	//!a simple time test
	void DiffTest();
	//!a simulated time test
	void SimulatedDiffTest();

protected:
	//--- subclass api

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

#endif
