/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "DiffTimer.h"
#include "Threads.h"

//--- interface include --------------------------------------------------------
#include "TestRunner.h"

//--- test cases ---------------------------------------------------------------
#include "SystemAPITest.h"
#include "MTStorageTest2.h"
#include "ThreadsTest.h"
#include "ThreadPoolTest.h"
#include "WorkerPoolManagerTest.h"
#include "WPMStatHandlerTest.h"
#include "LeaderFollowerPoolTest.h"

void setupRunner(TestRunner &runner)
{
	// !! ThreadsTest should be runned first, it tests synchronization and MT specific things which are
	// elementary things for the lib
	ADD_SUITE(runner, SystemAPITest);
	ADD_SUITE(runner, ThreadsTest);
	ADD_SUITE(runner, MTStorageTest2);
	ADD_SUITE(runner, ThreadPoolTest);
	ADD_SUITE(runner, WorkerPoolManagerTest);
	ADD_SUITE(runner, WPMStatHandlerTest);
	ADD_SUITE(runner, LeaderFollowerPoolTest);
} // setupRunner

