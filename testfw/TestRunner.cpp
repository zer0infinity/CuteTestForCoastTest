/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestRunner.h"

//--- standard modules used ----------------------------------------------------
#include "Test.h"
#include "TestList.h"
#include "TextTestResult.h"

//--- c-library modules used ---------------------------------------------------
#include <iostream>
#include <fstream>

#include <fcntl.h>
#if !defined(WIN32)
#include <unistd.h>
#else
#include <io.h>
#endif

/*
* Run all tests and collect their results.
*/

void TestRunner::usage(const char *progname)
{
	std::cout << "Usage: " << progname << " [Options] testNames ... " << std::endl
		 << "testName must be the name of a registered TestSuite "
		 << "(usually the name of a subclass of TestCase)." << std::endl
		 << "Options:" << std::endl
		 << "   -h         This help message" << std::endl
		 << "   -list      List the tests available in this program " << std::endl
		 << "   -log file  Write the successful tests to \"file\"" << std::endl
		 << "   -wait      Wait for user input for each subsequent test" << std::endl
		 << "   -all       Run all the available tests (plus the individually specified ones!)" << std::endl
		 << "   -out file  write stdout to file instead of usual stdout" << std::endl
		 << "   -err file  write stderr to file instead of usual stderr (aka /dev/null)" << std::endl;
} // usage

void TestRunner::listTests()
/* what: list the available tests of the program
*/
{
	std::cout << "Available Tests: " << std::endl;

	Test *test;
	for ( test = fMappings->first(); test != 0 ;  test = fMappings->next() ) {
		std::cout << "\t" << test->getClassName() << std::endl;
	}

} // listTests

void TestRunner::runAllTests()
/* what: run all tests in the mapping list in the order they where added
*/
{
	Test *test;
	for ( test = fMappings->first(); test != 0 ;  test = fMappings->next() ) {
		run(test->getSuite());
	}
} // listTests

void TestRunner::setLogToFileNamed(const char *name)
/* in: name: the name of the file to log into
 what: opens the named file and sets the stream
*/
{
	fLogStream = new std::fstream(name, std::ios::out);
} // setLogToFileNamed

TestRunner::TestRunner () : fWait(false), fLogStream(0),
	fNumberOfSuccesses(0), fNumberOfFailures(0), fElapsedTime()
{
	fMappings = new TestList;
}

// all comparisons done with upper case (mostly because of Open MVS
void TestRunner::run (int ac, const char **av)
{
	TString testCase;

	if (ac == 1) {
		usage(av[0]);
		return;
	} // if

	for (int i = 1; i < ac; ++i) {
		testCase = av [i];
		testCase.ToUpper();
		if ( testCase == "-H" ) {
			usage(av[0]);
			return;
		} else if ( testCase == "-ALL" ) {
			runAllTests();
			continue;
		} else if (testCase == "-LOG") {
			if (i < ac - 1) {
				setLogToFileNamed(av[++i]);
			} // if
			continue;
		} else if (testCase == "-OUT") {
			if (i < ac - 1) {
				++i;
				int outfile = open(av[i], O_RDWR | O_CREAT | O_TRUNC, 0660);
				if (outfile > 0) {	// make it stdout
					std::cout << "redirecting output to " << av[i] << std::endl;
					close(1);
					dup(outfile);	// ignore possible errors for now
					close(outfile);
				} else {
					std::cout << "output redirection failed" << std::endl;
				} // if
			} // if
			continue;
		} else if (testCase == "-ERR") {
			if (i < ac - 1) {
				++i;
				int outfile = open(av[i], O_RDWR | O_CREAT | O_TRUNC, 0660);
				if (outfile > 0) {	// make it stdout
					std::cerr << "redirecting errors to " << av[i] << std::endl;
					close(2);
					dup(outfile);	// ignore possible errors for now
					close(outfile);
				} else {
					std::cerr << "error redirection failed" << std::endl;
				} // if
			} // if
			continue;
		} else if (testCase == "-LIST") {
			listTests();
			continue;
		} else if (testCase == "-WAIT") {
			fWait = true;
			continue;
		} // if

		Test *test;
		for ( test = fMappings->first(); test != 0 ;
			  test = fMappings->next() ) {
			TString className = test->getClassName();
			className.ToUpper();

			if ( className == testCase ) {
				run(test->getSuite());
				break;
			} // if
		} // for

		if ( !test ) {
			std::cout << "Test " << testCase << " not found." << std::endl;
			return;
		}
	}
}

/*
 * Clean up
 */
TestRunner::~TestRunner ()
{
	delete fMappings;
	long testsRun = fNumberOfSuccesses + fNumberOfFailures;

	if (testsRun != 0) {
		std::cout << testsRun << " assertions run in "
			 << fElapsedTime.Diff() << " ms, " << fNumberOfFailures
			 << " failures; " << (fNumberOfSuccesses * 100) / testsRun
			 << "% complete" << std::endl;
	} // if

	if ( fLogStream != 0 ) {
		delete fLogStream;
	}
}

/*
 * Runs a single test and collects its results.
 */
void TestRunner::run (Test *test)
{
	TextTestResult	result;

	std::cout << "Running " << test->getClassName() << std::endl;

	test->run (&result);

	std::cout << result << std::endl;
	if (0 != fLogStream) {
		result.logSuccesses(*fLogStream);
	} // if

	if (fWait) {
		std::cout << "<RETURN> to continue" << std::endl;
		std::cin.get ();

	} // if
	fNumberOfSuccesses = fNumberOfSuccesses + result.successes().size();
	fNumberOfFailures = fNumberOfFailures + result.failures().size();
}

void TestRunner::addTest (const char *name, Test *suite)
/* in: name: the name of the TestSuite, caution, I will keep this pointer!
			 (i.e. make sure the name lives longer than the Test)
 what: sets the name of the test and adds it to the list
*/
{
	suite->setClassName(name);
	fMappings->push_back( suite );
} // TestRunner

long	TestRunner::getNumberOfFailures()
{
	return fNumberOfFailures;
}

long	TestRunner::getNumberOfSuccesses()
{
	return fNumberOfSuccesses;
}
