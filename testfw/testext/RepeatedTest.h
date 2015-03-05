
#ifndef CPP_UINT_REPEATEDTEST_H
#define CPP_UINT_REPEATEDTEST_H

#include "CppUnitCommon.h"
#include "TestDecorator.h"

/*
 * A decorator that runs a test repeatedly
 *
 */

class RepeatedTest : public TestDecorator
{
private:
	const int			m_timesRepeat;

public:
	RepeatedTest (Test *test, int repeat)
		: TestDecorator (test), m_timesRepeat (repeat) {}

	int					countTestCases ();
	TString				toString ();
	void				run (TestResult *result);

};

inline RepeatedTest::countTestCases ()
{
	return TestDecorator::countTestCases () * m_timesRepeat;
}

inline TString RepeatedTest::toString ()
{
	return TestDecorator::toString () + " (repeated)";
}

inline void RepeatedTest::run (TestResult *result)
{
	for (int n = 0; n < m_timesRepeat; n++) {
		if (result->shouldStop ()) {
			break;
		}
		TestDecorator::run (result);
	}
}

#endif
