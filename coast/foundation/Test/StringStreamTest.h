/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringStreamTest_h_
#define _StringStreamTest_h_

#include "TestCase.h"
//---- forward declaration -----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
class EXPORTDECL_FOUNDATION String;

//---- StringStreamTest -----------------------------------------------------------
//!testcases for StringStream
class StringStreamTest : public TestCase
{
public:
	StringStreamTest(TString name);
	virtual ~StringStreamTest();

	static Test *suite ();
	virtual void setUp ();
	void tearDown ();

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
