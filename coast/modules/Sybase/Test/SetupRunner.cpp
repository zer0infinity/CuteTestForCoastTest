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
#include "SybDBDAImplTest.h"
#include "SybDBTest.h"
#include "SetupCase.h"

#if defined(WIN32)
#include "config_bases.h"
void bases()
{
	RegisterBasesObjs();
}
#endif

void setupRunner(TestRunner &runner)
{
	// execute tests in test dir with defined structure
	ADD_SUITE(runner, SybDBTest);
	ADD_SUITE(runner, SybDBDAImplTest);
	ADD_SUITE(runner, ConfiguredActionTest);
}
