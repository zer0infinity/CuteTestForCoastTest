/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef IT_TESTFW_TestRunner_h_
#define IT_TESTFW_TestRunner_h_

#include "TestTimer.h"
#if defined(ONLY_STD_IOSTREAM)
#include <iosfwd>
using std::ostream;
#else
class ostream;
#endif
class Test;
class TestList;

#define ADD_SUITE(RUNNER,SUITE) \
	(RUNNER).addTest(#SUITE, SUITE::suite());

//---- TestRunner -----------------------------------------------------------
//! <B>main class running tests</B>
/*!
 * A command line based tool to run tests.
 * TestRunner expects as its only argument the name of a TestCase class.
 * TestRunner prints out a trace as the tests are executed followed by a
 * summary at the end.
 *
 * You can add to the tests that the TestRunner knows about by
 * making additional calls to "addTest (...)" in main.
 *
 * Here is the synopsis:
 *
 * TestRunner [-wait] ExampleTestCase
*/
class TestRunner
{
protected:
	bool		fWait;
	TestList	*fMappings;
// undef iostreams
#ifdef ostream
#define SAVEDOSTREAM 1
#undef ostream
#endif
	ostream		*fLogStream;
#if defined(SAVEDOSTREAM)
#undef SAVEDOSTREAM
#define ostream unsafe_ostream
#endif
	long		fNumberOfSuccesses;
	long		fNumberOfFailures;
	TestTimer	fElapsedTime;

public:
	TestRunner ();
	~TestRunner ();

	void		run (int ac, const char **av);
	void		addTest (const char *name, Test *suite);
	long		getNumberOfFailures();
	long		getNumberOfSuccesses();

protected:
	void		run (Test *test);
	void		usage(const char *progname);
	void		listTests();
	void		runAllTests();
	void		setLogToFileNamed(const char *name);
};

#endif
