/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------

//--- interface include --------------------------------------------------------
#include "TestRunner.h"

//--- test cases ---------------------------------------------------------------
#include "QueueWorkingModuleTest.h"
#include "QueueTest.h"

void setupRunner(TestRunner &runner)
{
	// add a whole suite with the ADD_SUITE(runner,"Suites's Classname") macro
	ADD_SUITE(runner, QueueTest);
	ADD_SUITE(runner, QueueWorkingModuleTest);
}

