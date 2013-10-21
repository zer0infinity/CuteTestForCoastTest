#ifndef AnythingIteratorTest_H
#define AnythingIteratorTest_H

#include "TestCase.h"//lint !e537
#include "Anything.h"//lint !e537
class AnythingIteratorTest: public testframework::TestCase {
protected:
	Anything fAny5; // anything with 5 elements set-up in setUp
public:
	AnythingIteratorTest(TString tname) :
		TestCaseType(tname) {
	}
	virtual void setUp();
	static Test *suite();

	void testEmptyAnythingBegin();
	void testSimpleAnythingBegin();
	void testSimpleAnythingDeref();
	void testSimpleAnythingIncrement();
	void testSimpleAnythingDecrement();
	void testSimpleAnythingIncDec();
	void testSimpleAnythingAssignment();
	void testAnythingIteration();
	void testAnythingIndex();
	void testIteratorSubstract();
	void testIteratorIntAdd();
	void testIteratorIntSubstract();
	void testIteratorCompare();
	void testAnythingSingleErase();
	void testAnythingRangeErase();
};

#endif
