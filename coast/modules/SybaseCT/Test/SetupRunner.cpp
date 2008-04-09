/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestRunner.h"

//--- test cases ---------------------------------------------------------------
#include "SybCTnewDATest.h"
#include "SybCTnewDAImplTest.h"
#include "ConfiguredActionTest.h"
#include "SybCTThreadTest.h"

void setupRunner(TestRunner &runner)
{
	// execute tests in test dir with defined structure
	ADD_SUITE(runner, SybCTnewDATest);
	ADD_SUITE(runner, SybCTnewDAImplTest);
	ADD_SUITE(runner, SybCTThreadTest);
	ADD_SUITE(runner, ConfiguredActionTest);
}
