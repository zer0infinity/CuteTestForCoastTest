/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TextTestResult.h"
//--- standard modules used ----------------------------------------------------

#if defined(ONLY_STD_IOSTREAM)
#include <iostream>
using namespace std;
#endif

void TextTestResult::addError (TestLocation *e)
{
	TestResult::addError (e);
	cerr << "E\n";

}

void TextTestResult::addFailure (TestLocation *e)
{
	TestResult::addFailure (e);
	cerr << "F\n";

}

void TextTestResult::startTest (Test *test)
{
	TestResult::startTest (test);
	cerr << ".";

}

void TextTestResult::printCauses(ostream &stream, TestLocList &causes)
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
		stream	<< i << ") " << "line: "
				<< cause->getFileName()
				<< ":"
				<< cause->getLineNumber()
				<< " "
				<< cause->getMessage()
				<< endl;
		i++;
	}
}

void TextTestResult::printErrors (ostream &stream)
{
	if (testErrors () != 0) {
		if (testErrors () == 1) {
			stream << "There was " << testErrors () << " error: " << endl;
		} else {
			stream << "There were " << testErrors () << " errors: " << endl;
		}
		// if

		printCauses(stream, errors());
	} // if
}

void TextTestResult::logSuccesses(ostream &stream)
/* in: stream: the stream to write all the successes to
 what: writes all successes on a stream, at some point the format may be parseable
*/
{
	stream << testSuccesses() << " assertions completed successfully:" << endl;

	printCauses(stream, successes());
} // logSuccesses

void TextTestResult::printFailures (ostream &stream)
{
	if ( testFailures () != 0 ) {
		if (testFailures () == 1) {
			stream << "There was " << testFailures () << " failure: " << endl;
		} else {
			stream << "There were " << testFailures () << " failures: " << endl;
		}

		printCauses(stream, failures());
	}
}

void TextTestResult::print (ostream &stream)
{
	printHeader (stream);
	printErrors (stream);
	printFailures (stream);

}

void TextTestResult::printHeader (ostream &stream)
{
	if (wasSuccessful ())
		cout << endl << "OK (" << runTests () << " tests and "
			 << testSuccesses() << " assertions in " << elapsedTime() << " ms)" << endl;
	else
		cout << endl
			 << "!!!FAILURES!!!" << endl
			 << "Test Results:" << endl
			 << "Run:  "
			 << runTests ()
			 << "   Failures: "
			 << testFailures ()
			 << "   Errors: "
			 << testErrors ()
			 << endl
			 << "(" << testSuccesses() << " assertions ran successfully in " << elapsedTime() << " ms)" << endl;

}
