
#ifndef CPP_UNIT_TESTDECORATOR_H
#define CPP_UNIT_TESTDECORATOR_H

#include "CppUnitCommon.h"
#include "Test.h"

class TestDecorator : public Test
{
protected:
	Test		*m_test;

public:
	TestDecorator (Test *test);
	~TestDecorator ();
	int			countTestCases ();
	void		run (TestResult *result);
	TString		toString ();

};

inline TestDecorator::TestDecorator (Test *test)
{
	m_test = test;
}

inline TestDecorator::~TestDecorator ()
{
	delete m_test;
}

inline TestDecorator::countTestCases ()
{
	return m_test->countTestCases ();
}

inline void TestDecorator::run (TestResult *result)
{
	m_test->run (result);
}

inline TString TestDecorator::toString ()
{
	return m_test->toString ();
}

#endif
