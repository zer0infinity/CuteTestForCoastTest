#ifndef StringIteratorTest_H
#define StringIteratorTest_H

#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"
#include "ITOString.h"//lint !e537

class StringIteratorTest {
protected:
	String fStr5; // string with 5 elements set-up in setUp
public:
	void runAllTests(int argc, char const *argv[]);
	StringIteratorTest();

	void testEmptyStringBegin();
	void testSimpleStringBegin();
	void testSimpleStringDeref();
	void testSimpleStringIncrement();
	void testSimpleStringDecrement();
	void testSimpleStringIncDec();
	void testSimpleStringAssignment();
	void testStringIteration();
	void testStringIndex();
	void testIteratorSubstract();
	void testIteratorIntAdd();
	void testIteratorIntSubstract();
	void testIteratorCompare();
};

#endif
