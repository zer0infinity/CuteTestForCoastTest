/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "a2ee2a.h"

//--- interface include --------------------------------------------------------
#include "EBCDICTableTest.h"

EBCDICTableTest::EBCDICTableTest(TString tname) : TestCase(tname) {}

EBCDICTableTest::~EBCDICTableTest() {};

void EBCDICTableTest::setUp ()
{
	TestCase::setUp();
};

void EBCDICTableTest::WholeTable ()
{
	const long numChars = 256;
	unsigned char toConvert[numChars];
	unsigned char converted[numChars];
	unsigned char backConverted[numChars];

	long i;
	for (i = 0; i < numChars; i++) {
		toConvert[i] = (unsigned char) i;
	} // for

	ascii2ebcdic(converted, toConvert, numChars);
	ebcdic2ascii(backConverted, converted, numChars);

	for (i = 0; i < numChars; i++) {
		assertEqual(i, (long) backConverted[i]);
	} // for

} // WholeTable

Test *EBCDICTableTest::suite ()
/* what: return the whole suite of tests for EBCDICTableTest, add all top level
		 test functions here.
*/
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, EBCDICTableTest, WholeTable);

	return testSuite;
} // suite
