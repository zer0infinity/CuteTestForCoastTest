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

#ifndef _StringStreamTest_h_
#define _StringStreamTest_h_

#include "cute.h"

class String;

class StringStreamTest
{
public:
	static void runAllTests(cute::suite &s);

	void FormatTests();
	void SimpleWrite();
	void SimpleRead();
	void SimpleAppend();
	void SimpleAtEnd();
	void SimpleSeek();
	void SimpleOStringStreamWriteTest();
	void WriteAnyOnString();
	void ReadFromAndWriteToStringTest();
	void ReadFromAndWriteToAnythingTest();
	void ReadFromAndWriteToAnythingTest2();
	void OperatorShiftLeftWithReadBuf();
	void OperatorShiftLeftWithUnsignedLongLong();
	void OperatorShiftLeftWithLongLong();
	void OperatorShiftRightWithLongLong();
	void OperatorShiftRightWithUnsignedLongLong();

	static const char *const fgcContent;

protected:
	void testformat(const String &source, long expect, bool mustfail, const String &msg);
	void testformat(const String &source, double expect, bool mustfail, const String &msg);
};

#endif
