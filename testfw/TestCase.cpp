/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestCase.h"
//--- class used include ------------------------------------------------------
#include "TestResult.h"

/* run the test and catch any exceptions that are triggered by it */
void TestCase::run (TestResult *result)
{
	Test::run(result);
	result->startTest (this);

	setUp ();

	runTest ();

	tearDown ();
	result->endTest (this);
}

/* A default run method */
TestResult *TestCase::run ()
{
	TestResult *result = new TestResult;

	run (result);
	return result;
}

/* All the work for runTest is deferred to subclasses */
void TestCase::runTest ()
{
	(this->*fTheTest)();
}

TestCase *TestCase::setTheTest(CaseMemberPtr p_TheTest)
{
	fTheTest = p_TheTest;
	return this;
}
