#ifndef _TracerTest_H
#define _TracerTest_H

#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"

class TracerTest {
public:
	TracerTest();
	virtual ~TracerTest();
	void runAllTests(int argc, char const *argv[]);

	void TracerTestExplicitlyEnabled();
	void TracerTestLowerBoundZero();
	void TracerTestEnableAllFirstLevel();
	void TracerTestEnableAllSecondLevel();
	void TracerTestEnableAllThirdLevel();
	void TracerTestEnableAllAboveUpperBound();
	void TracerTestEnableAllBelowLowerBound();
	void TracerTestEnableAllSecondAndBelowDisabled();
	void TracerTestNotAllAboveLowerBound();
	void TracerTestBug248();

	//!test case for the Tracer.h test macros
	void CheckMacrosCompile();
};

#endif
