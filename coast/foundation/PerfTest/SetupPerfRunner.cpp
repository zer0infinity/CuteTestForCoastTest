/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestRunner.h"
#include "AnythingPerfTest.h"
#include "StringPerfTest.h"

//--- standard modules used ----------------------------------------------------

void setupRunner(TestRunner &runner)
{//lint !e14
	ADD_SUITE(runner, StringPerfTest);
	ADD_SUITE(runner, AnythingPerfTest);
}
