/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ConversionUtilsTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ConversionUtils.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include <iostream>

//--- c-modules used -----------------------------------------------------------

//---- ConversionUtilsTest ----------------------------------------------------------------
ConversionUtilsTest::ConversionUtilsTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(ConversionUtilsTest.Ctor);
}

ConversionUtilsTest::~ConversionUtilsTest()
{
	StartTrace(ConversionUtilsTest.Dtor);
}

#define TestSpecificValue(valType, value, from, order) \
	{ valType myX = 0; \
	ConversionUtils::GetValueFromBuffer(strBuf, myX, from, order); \
	assertEqual((valType)value, myX); }

void ConversionUtilsTest::GetValueFromBufferTest()
{
	StartTrace(ConversionUtilsTest.GetValueFromBufferTest);
	String strBuf("\x11\x22\x33\x44\x99\xaa\xbb\xcc");
	// ordered LSB2MSB
	TestSpecificValue(ul_long, UINT64_LITERAL(0xccbbaa9944332211), 0L, ConversionUtils::eLSB2MSB);
	TestSpecificValue(l_long, INT64_LITERAL(0xccbbaa9944332211), 0L, ConversionUtils::eLSB2MSB);
	TestSpecificValue(unsigned long, 0x44332211UL, 0L, ConversionUtils::eLSB2MSB);
	TestSpecificValue(long, 0xccbbaa99, 4L, ConversionUtils::eLSB2MSB);
	TestSpecificValue(unsigned int, 0x44332211, 0L, ConversionUtils::eLSB2MSB);
	TestSpecificValue(int, 0xccbbaa99, 4L, ConversionUtils::eLSB2MSB);
	TestSpecificValue(unsigned short, 0x2211, 0L, ConversionUtils::eLSB2MSB);
	TestSpecificValue(short, 0xccbb, 6L, ConversionUtils::eLSB2MSB);
	TestSpecificValue(unsigned char, 0x11, 0L, ConversionUtils::eLSB2MSB);
	TestSpecificValue(char, 0xcc, 7L, ConversionUtils::eLSB2MSB);
	// ordered MSB2LSB
	TestSpecificValue(ul_long, UINT64_LITERAL(0x1122334499aabbcc), 0L, ConversionUtils::eMSB2LSB);
	TestSpecificValue(l_long, INT64_LITERAL(0x1122334499aabbcc), 0L, ConversionUtils::eMSB2LSB);
	TestSpecificValue(unsigned long, 0x11223344UL, 0L, ConversionUtils::eMSB2LSB);
	TestSpecificValue(long, 0x99aabbcc, 4L, ConversionUtils::eMSB2LSB);
	TestSpecificValue(unsigned int, 0x11223344, 0L, ConversionUtils::eMSB2LSB);
	TestSpecificValue(int, 0x99aabbcc, 4L, ConversionUtils::eMSB2LSB);
	TestSpecificValue(unsigned short, 0x1122, 0L, ConversionUtils::eMSB2LSB);
	TestSpecificValue(short, 0xbbcc, 6L, ConversionUtils::eMSB2LSB);
	TestSpecificValue(unsigned char, 0x11, 0L, ConversionUtils::eMSB2LSB);
	TestSpecificValue(char, 0xcc, 7L, ConversionUtils::eMSB2LSB);
}

// builds up a suite of testcases, add a line for each testmethod
Test *ConversionUtilsTest::suite ()
{
	StartTrace(ConversionUtilsTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ConversionUtilsTest, GetValueFromBufferTest);

	return testSuite;
}
