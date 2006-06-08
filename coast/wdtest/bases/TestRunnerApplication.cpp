/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestRunnerApplication.h"

//--- module under test --------------------------------------------------------
#include "TestRunner.h"
#include "SetupRunner.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

RegisterApplication(TestRunnerApplication);

TestRunnerApplication::TestRunnerApplication()
	: Application("TestRunnerApplication")
{ }

TestRunnerApplication::TestRunnerApplication(const char *AppName)
	: Application(AppName)
	, fgArgc(0)
	, fgArgv(0)
{ }

TestRunnerApplication::~TestRunnerApplication()
{
}

// GlobalInit: keeps argc and argv for later use
int TestRunnerApplication::GlobalInit(int argc, char *argv[], const ROAnything config)
{
	StartTrace(TestRunnerApplication.GlobalInit);
	fgArgc = argc;
	fgArgv = argv;

	return Application::GlobalInit(argc, argv, config);	// Call instance init
}

// doing the work
int TestRunnerApplication::Run()
{
	StartTrace(TestRunnerApplication.Run);
	TestRunner runner;

	setupRunner(runner);
	runner.run (fgArgc, fgArgv);

	return runner.getNumberOfFailures();
}
