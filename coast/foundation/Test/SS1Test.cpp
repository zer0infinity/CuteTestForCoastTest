/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SS1Test.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- c-library modules used ---------------------------------------------------

SS1Test::SS1Test(TString tname) : TestCase(tname)
{};

SS1Test::~SS1Test() {}

void SS1Test::setUp ()
{
	TestCase::setUp();
};

Test *SS1Test::suite ()
/* what: return the suite of tests
*/
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SS1Test, SimpleTest);
	return testSuite;
}

void SS1Test::SimpleTest()
// what: some very simplistic things to find out what does not work about flush
{
	String out;
	{
		OStringStream os(&out);

		os << "something very small" << flush;
		assertEqual("something very small", out);
		assertEqual(32, out.Capacity());
		assertEqual(20, out.Length());
	}
	String out2("something very small");
	assertEqual("something very small", out2);
	assertEqual(21, out2.Capacity());
	assertEqual(20, out2.Length());

	out << " a little bit more, this should now double the string";
	assertEqual("something very small a little bit more, this "
				"should now double the string", out);
	assertEqual(146, out.Capacity());
	assertEqual(73, out.Length());
} // SimpleTest

