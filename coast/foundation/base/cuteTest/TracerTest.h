/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TracerTest_H
#define _TracerTest_H

#include "cute.h"

class TracerTest {
public:
	TracerTest();
	virtual ~TracerTest();
	static void runAllTests(cute::suite &s);

	void TracerTestExplicitlyEnabled();
	void TracerTestLowerBoundZero();
	void TracerTestEnableAllFirstLevel();
	void TracerTestEnableAllSecondLevel();
	void TracerTestEnableAllThirdLevel();
	void TracerTestEnableAllAboveUpperBound();
	void TracerTestEnableAllBelowLowerBound();
	void TracerTestEnableAllSecondAndBelowDisabled();
	void TracerTestNotAllAboveLowerBound();
	void TracerTestBug248();

	//!test case for the Tracer.h test macros
	void CheckMacrosCompile();
};

#endif
