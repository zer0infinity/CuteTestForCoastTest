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
#include "SybCTTest.h"
#include "SybCTDAImplTest.h"
#include "SybCTnewDATest.h"
#include "SybCTnewDAImplTest.h"
#include "SybCTPoolDAImplTest.h"
#include "ConfiguredActionTest.h"
#include "SybCTThreadTest.h"
#if !defined(WIN32)
#include "SetupCase.h"
#endif

void setupRunner(TestRunner &runner)
{
	// execute tests in test dir with defined structure
#if !defined(WIN32)
	ADD_SUITE(runner, SetupCase);
#endif
	ADD_SUITE(runner, SybCTTest);
	ADD_SUITE(runner, SybCTDAImplTest);
	ADD_SUITE(runner, SybCTnewDATest);
	ADD_SUITE(runner, SybCTnewDAImplTest);
	ADD_SUITE(runner, SybCTPoolDAImplTest);
	ADD_SUITE(runner, SybCTThreadTest);
	ADD_SUITE(runner, ConfiguredActionTest);
}
