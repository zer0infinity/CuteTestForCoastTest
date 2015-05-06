/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef AnythingIteratorTest_H
#define AnythingIteratorTest_H

#include "cute.h"
#include "Anything.h"//lint !e537
class AnythingIteratorTest {
protected:
	Anything fAny5; // anything with 5 elements set-up in setUp
public:
	AnythingIteratorTest();
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
