/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestRunner.h"

//--- standard modules used ----------------------------------------------------

//--- test cases ---------------------------------------------------------------
#include "ConfiguredActionTest.h"
#include "CopyActionsTest.h"

#if defined(WIN32)
#include "config_bases.h"
void bases()
{
	RegisterBasesObjs();
}
#endif

void setupRunner(TestRunner &runner)
{
	// add a whole suite with the ADD_SUITE(runner,"Suites's Classname") macro
	ADD_SUITE(runner, ConfiguredActionTest);
	ADD_SUITE(runner, CopyActionsTest);
} // setupRunner
