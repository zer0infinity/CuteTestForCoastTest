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

#ifndef _AnythingTest_H
#define _AnythingTest_H

#include "cute.h"
#include "SystemFile.h"
#include "AssertionAnything.h"

class AnythingTest : public Assertion {
protected:
	Anything fQuery;
	Anything fConfig;
public:
	AnythingTest();
	static void runAllTests(cute::suite &s);
	void TypeTest();
	void SuccessiveAssignments();

	void operatorAssignemnt();
	void appendTest();
	void boolOperatorAssign();
	void intOperatorAssign();
	void ifaObjectOperatorAssign();

	void roConversion();
	void boolROConversion();
	void intROConversion();
	void ifaObjectROConversion();
	void BinaryBufOutput();
	void String2LongConversion();

	void AsCharPtrBufLen();
	void RefCount();

	void SlotFinderTest();
	void SlotCopierTest();
	void SlotPutterTest();
	void SlotPutterAppendTest();
	void AnythingLeafIteratorTest();
	void SlotnameSorterTest();
	void SlotCleanerTest();
};

#endif		//ifndef _AnythingTest_H
