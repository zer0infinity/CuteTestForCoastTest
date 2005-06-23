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
#include "MD5Test.h"
#include "BlowfishTest.h"
#include "Base64Test.h"
#include "ScrambleStateTest.h"
#include "TableCompressorTest.h"
#include "NewRendererTest.h"

void setupRunner(TestRunner &runner)
{
	// execute tests in test dir with defined structure
	ADD_SUITE(runner, Base64Test);
	ADD_SUITE(runner, MD5Test);
	ADD_SUITE(runner, BlowfishTest);
	ADD_SUITE(runner, ScrambleStateTest);
	ADD_SUITE(runner, TableCompressorTest);
	ADD_SUITE(runner, NewRendererTest);
} // setupRunner
