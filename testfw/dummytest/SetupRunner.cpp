/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestRunner.h"
#include "DummyTest.h"
#include "AssertionTest.h"

void setupRunner(TestRunner &runner)
{
	// add a whole suite with the ADD_SUITE(runner,"Suites's Classname") macro
	ADD_SUITE(runner, DummyTest);
	ADD_SUITE(runner, AssertionTest);
}

