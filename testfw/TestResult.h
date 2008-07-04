/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef IT_TESTFW_TESTRESULT_H
#define IT_TESTFW_TESTRESULT_H

#include "TestLocation.h"
#include "TestLocList.h"
#include "TestTimer.h"

class Test;

//! <B>collect the results of the tests</B>
/*!
 * A TestResult collects the results of executing a test case. It is an
 * instance of the Collecting Parameter pattern.
 * The test framework distinguishes between failures and errors.
 * A failure is anticipated and checked for with assertions. Errors are
 * unanticipated problems signified by exceptions that are not generated
 * by the framework.
 *
 * see Test
 */
class TestResult
{
public:
	TestResult  ();
	virtual						~TestResult () {};

	virtual bool				shouldStop	();
	virtual void				addError	(TestLocation *loc);
	virtual void				addFailure	(TestLocation *loc);
	virtual void				addSuccess	(TestLocation *loc);
	virtual void				startTest	(Test *test);
	virtual void				endTest		(Test *test);
	virtual int					runTests	();
	virtual long				elapsedTime	();
	virtual int					testErrors  ();
	virtual int					testFailures ();
	virtual int					testSuccesses ();
	virtual bool				wasSuccessful ();
	virtual void				stop ();

	TestLocList		&errors ();
	TestLocList		&failures ();
	TestLocList		&successes ();

protected:
	TestLocList		fErrors;
	TestLocList		fFailures;
	TestLocList		fSuccesses;
	int				fRunTests;
	TestTimer		fTimer;
	long			fElapsedTime;

private:
	bool						m_stop;
};

inline TestResult::TestResult (): fRunTests(0), fElapsedTime(0)
{
	m_stop = false;
}

// Returns whether the test should stop
inline bool TestResult::shouldStop ()
{
	return m_stop;
}

// Adds an error to the list of errors. The passed in exception
// caused the error
inline void TestResult::addError (TestLocation *loc)
{
	fErrors.push_back (loc);
}

// Adds a failure to the list of failures. The passed in exception
// caused the failure.
inline void TestResult::addFailure (TestLocation *loc)
{
	fFailures.push_back (loc);
}

// Adds a success to the list of successes. The passed in exception
// describes the test.
inline void TestResult::addSuccess (TestLocation *loc)
{
	fSuccesses.push_back (loc);
}

// Informs the result that a test will be started.
inline void TestResult::startTest (Test *test)
{
	++fRunTests;
	fTimer.Start();
}

// Informs the result that a test was completed.
inline void TestResult::endTest (Test *test)
{
	fElapsedTime += fTimer.Diff();
}

// Gets the number of run tests.
inline int TestResult::runTests ()
{
	return fRunTests;
}

// Gets the number of run tests.
inline long TestResult::elapsedTime ()
{
	return fElapsedTime;
}

// Gets the number of detected errors.
inline int TestResult::testErrors ()
{
	return fErrors.size ();
}

// Gets the number of detected failures.
inline int TestResult::testFailures ()
{
	return fFailures.size ();
}

// Gets the number of successes.
inline int TestResult::testSuccesses ()
{
	return fSuccesses.size ();
}

// Returns whether the entire test was successful or not.
inline bool TestResult::wasSuccessful ()
{
	return fFailures.size () == 0 && fErrors.size () == 0;
}

// Marks that the test run should stop.
inline void TestResult::stop ()
{
	m_stop = true;
}

// Returns a vector of the errors.
//inline vector<TestLocation *>& TestResult::errors ()
inline TestLocList &TestResult::errors ()
// be careful with these, you will get a reference, but I take care of memory
{
	return fErrors;
}

// Returns a vector of the failures.
//inline vector<TestLocation *>& TestResult::failures ()
inline TestLocList &TestResult::failures ()
{
	return fFailures;
}

// Returns a vector of the successes.
//inline vector<TestLocation *>& TestResult::successes ()
inline TestLocList &TestResult::successes ()
{
	return fSuccesses;
}

#endif
