
#ifndef CPP_UINT_TESTSETUP_H
#define CPP_UINT_TESTSETUP_H

#include "CppUnitCommon.h"
#include "TestDecorator.h"

class TestSetup : public TestDecorator
{
public:
	TestSetup (Test *test) : TestDecorator (test) {}
	run (TestResult *result);

protected:
	void	setUp () {}
	void	tearDown () {}

};

inline TestSetup::run (TestResult *result)
{
	setUp ();
	TestDecorator::run (result);
	tearDown ();
}

#endif
