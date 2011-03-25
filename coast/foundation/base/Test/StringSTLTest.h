/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringSTLTest_h_
#define _StringSTLTest_h_

#include "TestCase.h"//lint !e537
#include "ITOString.h"//lint !e537

//---- StringSTLTest -----------------------------------------------------------
//!some special tests for String
class StringSTLTest: public TestFramework::TestCase {
	String fStr5; // string with 5 elements set-up in setUp
public:
	StringSTLTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	virtual void setUp();
	static Test *suite();

	//-- test String's support for iterators:
	void testStringSingleErase();
	void testStringRangeErase();
	void testStringRangeIteratorErase();
	void testStringInsertIteratorSingle();
	void testStringInsertIteratorMultiple();
	void testStringInsertIteratorRange();
};

#endif
