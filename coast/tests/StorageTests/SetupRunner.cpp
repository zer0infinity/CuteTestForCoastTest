/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestRunner.h"
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------

//--- test cases -------------------------------------------------------------
#include "MemTrackerTest.h"
#include "StorageTest.h"
#include "PoolAllocatorTest.h"
#include "STLStorageTest.h"
#include "BoostPoolTest.h"

void setupRunner(TestRunner &runner)
{
	ADD_SUITE(runner, MemTrackerTest);
	ADD_SUITE(runner, StorageTest);
	ADD_SUITE(runner, PoolAllocatorTest);
	ADD_SUITE(runner, STLStorageTest);
	ADD_SUITE(runner, BoostPoolTest);
}