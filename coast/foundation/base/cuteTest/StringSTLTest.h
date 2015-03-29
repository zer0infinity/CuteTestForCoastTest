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

#ifndef _StringSTLTest_h_
#define _StringSTLTest_h_

#include "cute.h"
#include "ITOString.h"//lint !e537

//!some special tests for String
class StringSTLTest {
	String fStr5; // string with 5 elements set-up in setUp
public:
	StringSTLTest();
	static void runAllTests(cute::suite &s);

	//-- test String's support for iterators:
	void testStringSingleErase();
	void testStringRangeErase();
	void testStringRangeIteratorErase();
	void testStringInsertIteratorSingle();
	void testStringInsertIteratorMultiple();
	void testStringInsertIteratorRange();
};

#endif
