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

#ifndef AnythingSTLTest_H
#define AnythingSTLTest_H

#include "cute.h"//lint !e537
#include "Anything.h"//lint !e537
class AnythingSTLTest {
	void checkRange(const Anything &, long n, long length);
	void checkFill(const Anything &);
	void checkFillSizeType(const Anything &);
	void checkSimpleInsert(const Anything &, const char *msg);
	void checkInsertInArray(const Anything &a, long testpos, const char *msg, long n = 1);
	void checkInsertWithKeys(const Anything &a, const long testpos, const char *m, const long n = 1);

public:
	static void runAllTests(cute::suite &s);

	void testSimpleSwap();
	void testSwapWithDifferentAllocator();
	void testFrontBackPushPop();
	void testRangeAssign();
	void testFillAssign();
	void testFillAssignWithSizeType();
	void testRangeCtor();
	void testFillCtor();
	void testFillCtorWithSizeType();

	void testSimpleInsertToEmptyAny();
	void testSimpleInsertAtEnd();
	void testSimpleInsertAtBegin();
	void testSimpleInsertInArray();
	void testSimpleInsertWithKeys();

	void testFillInsertToEmptyAny();
	void testFillInsertInArray();
	void testFillInsertWithKeys();
	void testFillInsertZero();

	void testIterInsertToEmptyAny();
	void testIterInsertInArray();
	void testIterInsertWithKeys();
	void testIterInsertZero();
};
#endif
