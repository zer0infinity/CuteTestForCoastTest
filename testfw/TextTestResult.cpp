/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TextTestResult.h"
#include <iostream>

void TextTestResult::addError (TestLocation *loc)
{
	TestResult::addError (loc);
	std::cerr << "E";
}

void TextTestResult::addFailure (TestLocation *loc)
{
	TestResult::addFailure (loc);
	std::cerr << "F";
}

void TextTestResult::startTest (Test *test)
{
	TestResult::startTest (test);
	fStartTime = elapsedTime();
	std::cerr << "--" << test->toString() << "--";
}

void TextTestResult::endTest (Test *test)
{
	TestResult::endTest (test);
	std::cerr << " (" << (elapsedTime() - fStartTime) << "ms)";
	std::cerr << std::endl;
}

void TextTestResult::printCauses(std::ostream &stream, TestLocList &causes)
/* in: stream: where to write
	   causes: what to write
 what: print the causes sequentially;
	   refactoring of printErrors and printFailures
 Caution: TestLocList must not be copied here
*/
{
	int i = 1;

	TestLocation *cause;
	for ( cause = causes.first(); cause != 0; cause = causes.next() ) {
		stream	<< i << ") " << cause->getTestName() << ": "
				<< cause->getFileName()
				<< ":"
				<< cause->getLineNumber()
				<< ": "
				<< cause->getMessage()
				<< std::endl;
		++i;
	}
}

void TextTestResult::printErrors (std::ostream &stream)
{
	if (testErrors () != 0) {
		if (testErrors () == 1) {
			stream << "There was " << testErrors () << " error: " << std::endl;
		} else {
			stream << "There were " << testErrors () << " errors: " << std::endl;
		}
		// if

		printCauses(stream, errors());
	} // if
}

void TextTestResult::logSuccesses(std::ostream &stream)
/* in: stream: the stream to write all the successes to
 what: writes all successes on a stream, at some point the format may be parseable
*/
{
	stream << testSuccesses() << " assertions completed successfully:" << std::endl;

	printCauses(stream, successes());
} // logSuccesses

void TextTestResult::printFailures (std::ostream &stream)
{
	if ( testFailures () != 0 ) {
		if (testFailures () == 1) {
			stream << "There was " << testFailures () << " failure: " << std::endl;
		} else {
			stream << "There were " << testFailures () << " failures: " << std::endl;
		}

		printCauses(stream, failures());
	}
}

void TextTestResult::print (std::ostream &stream)
{
	printHeader (stream);
	printErrors (stream);
	printFailures (stream);

}

void TextTestResult::printHeader (std::ostream &stream)
{
	if (wasSuccessful ())
		std::cout << "OK (" << runTests () << " tests and "
			 << testSuccesses() << " assertions in " << elapsedTime() << " ms)" << std::endl;
	else
		std::cout << std::endl
			 << "!!!FAILURES!!!" << std::endl
			 << "Test Results:" << std::endl
			 << "Run:  "
			 << runTests ()
			 << "   Failures: "
			 << testFailures ()
			 << "   Errors: "
			 << testErrors ()
			 << std::endl
			 << "(" << testSuccesses() << " assertions ran successfully in " << elapsedTime() << " ms)" << std::endl;

}
