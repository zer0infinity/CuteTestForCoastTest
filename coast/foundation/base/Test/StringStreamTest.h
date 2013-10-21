/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringStreamTest_h_
#define _StringStreamTest_h_

#include "TestCase.h"//lint !e537

class String;

class StringStreamTest : public testframework::TestCase
{
public:
	StringStreamTest(TString tstrName);
	virtual ~StringStreamTest();

	static Test *suite ();

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
	void testformat(const String &source, long expect, bool mustfail, const TString &msg);
	void testformat(const String &source, double expect, bool mustfail, const TString &msg);
};

#endif
