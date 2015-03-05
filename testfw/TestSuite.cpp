/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "TestList.h"
#include "TestResult.h"

TestSuite::TestSuite ()
{
	fTests = new TestList;
}
// Deletes all tests in the suite.
TestSuite::~TestSuite ()
{
	delete fTests;
}

void TestSuite::addTest (Test *test)
{
	fTests->push_back (test);
}

// Runs the tests and collects their result in a TestResult.
void TestSuite::run (TestResult *result)
{
	Test::run(result);

	Test *test;
	for ( test = fTests->first(); test != 0 ;  test = fTests->next() ) {
		if (result->shouldStop ()) {
			break;
		}
		test->run (result);
	}
}

// Counts the number of test cases that will be run by this test.
int TestSuite::countTestCases ()
{
	int count = 0;

	Test *test;
	for ( test = fTests->first(); test != 0 ;  test = fTests->next() ) {
		count += test->countTestCases ();
	}

	return count;
}
