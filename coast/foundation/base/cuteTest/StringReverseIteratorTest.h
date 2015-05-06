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

#ifndef StringReverseIteratorTest_H
#define StringReverseIteratorTest_H

#include "cute.h"
#include "ITOString.h"//lint !e537

class StringReverseIteratorTest {
protected:
	String fStr5; // string with 5 elements set-up in setUp
public:
	StringReverseIteratorTest();
	static void runAllTests(cute::suite &s);

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
