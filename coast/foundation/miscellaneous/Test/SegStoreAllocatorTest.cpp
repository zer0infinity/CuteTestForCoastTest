/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SegStoreAllocatorTest.h"
#include "TestSuite.h"
#include "Dbg.h"
#include "Anything.h"
#include "SegStoreAllocator.h"

void SegStoreAllocatorTest::CreateSimpleAllocator() {
	StartTrace(SegStoreAllocatorTest.CreateSimpleAllocator);
	SegStoreAllocator p(11223344L);
	Anything a(&p);

	a.Append("Test");
	t_assertm(true, "dummy assertion to generate summary output");
}

Test *SegStoreAllocatorTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SegStoreAllocatorTest, CreateSimpleAllocator);

	return testSuite;
}
