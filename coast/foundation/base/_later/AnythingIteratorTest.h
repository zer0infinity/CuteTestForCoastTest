#ifndef AnythingIteratorTest_H
#define AnythingIteratorTest_H

#include "cute.h"
#include "Anything.h"//lint !e537
class AnythingIteratorTest {
protected:
	Anything fAny5; // anything with 5 elements set-up in setUp
public:
	AnythingIteratorTest() {
	}
	static void runAllTests(cute::suite &s);

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
