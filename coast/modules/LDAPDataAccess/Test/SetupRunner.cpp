/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"

//--- interface include --------------------------------------------------------
#include "TestRunner.h"

//--- test cases ---------------------------------------------------------------
#include "ConfiguredActionTest.h"
#include "LDAPMapperTest.h"

void setupRunner(TestRunner &runner)
{
	// Use this work suite to debug a failing TestCase
	// insert your work to test here
	ADD_SUITE(runner, LDAPMapperTest);
	ADD_SUITE(runner, ConfiguredActionTest);
} // setupRunner
